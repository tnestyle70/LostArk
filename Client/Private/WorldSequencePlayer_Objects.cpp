#include "WorldSequencePlayer.h"
#include "WorldSequenceObject.h"
#include "DeployPropObject.h"
#include "GameInstance.h"
#include "Model.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "RuntimeAssetRoot.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace Client;
using namespace Engine;

CWorldSequencePlayer::~CWorldSequencePlayer() { Clear(); }

void CWorldSequencePlayer::Collect_ValidationTargets(const TARGET_SET& targets,
    WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy)
{
    placements.clear(); deploy.clear();
    if (!targets.Is_Complete()) return;
    // Load the complete placement source, not only the current rendering scope.
    std::vector<MAP_PLACEMENT_RECORD> records;
    std::string status;
    if (CMapPlacementRuntime::Read_Placements(*targets.pCatalog, records, status))
        for (const auto& row : records)
        {
            const auto* asset = targets.pCatalog->Find(row.assetId);
            placements.emplace(row.placementId, WORLD_SEQUENCE_PLACEMENT_INFO{row.signedScale,
                asset && asset->renderProfile.renderMode != MAP_ASSET_RENDER_MODE::BACKGROUND});
        }
    for (const auto& entry : *targets.pPlacements)
    {
        const auto* asset = targets.pCatalog->Find(entry.record.assetId);
        placements.insert_or_assign(entry.record.placementId, WORLD_SEQUENCE_PLACEMENT_INFO{
            entry.record.signedScale, asset && asset->renderProfile.renderMode != MAP_ASSET_RENDER_MODE::BACKGROUND});
    }
    for (const auto& entry : targets.pDeployRuntime->Get_Entries())
    {
        WORLD_SEQUENCE_DEPLOY_INFO info;
        if (entry.object && !entry.object->Is_StaticDeployModel())
        {
            for (const auto& clip : entry.object->Get_AnimationClips()) info.animationClips.push_back(clip.name);
            info.animationTargetSupported = !info.animationClips.empty();
        }
        deploy.emplace(entry.placement.runtimePlacementId, std::move(info));
    }
}

bool_t CWorldSequencePlayer::Set_Document(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, std::string& status)
{
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    Collect_ValidationTargets(targets, placements, deploy);
    if (!targets.Is_Complete())
    { m_Status = status = "World Object runtime targets are unavailable."; return false; }
    if (!document.Validate(placements, deploy, status))
    { m_Status = status; return false; }
    Stop_All(targets, true);
    m_ObjectModels.clear();
    m_EffectSnapshots.clear();
    m_Document = document;
    m_Status = status = "World Object document admitted.";
    return true;
}

bool_t CWorldSequencePlayer::Prepare_ObjectResources(
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    const auto* sequence = m_Document.Find_Template(instance.templateId);
    if (sequence)
        for (const auto& effect : sequence->effectTracks)
        {
            const auto key = effect.resourceKind + ":" + effect.resourceId;
            if (m_EffectSnapshots.contains(key)) continue;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
            std::string error;
            const auto kind = effect.resourceKind == "GROUP" ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF;
            if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(kind, effect.resourceId, snapshot, error))
            { m_Status = "World Object effect admission failed: " + effect.resourceId + " / " + error; return false; }
            EFFECT_V2_GROUP group;
            if (kind == EFFECT_V2_RESOURCE_KIND::GROUP) group = *snapshot->Find_Group(effect.resourceId);
            else
            {
                group.strGroupId = effect.resourceId;
                EFFECT_V2_GROUP_CHILD child;
                child.strChildId = "object.effect.leaf"; child.strResourceId = child.strEffectId = effect.resourceId;
                group.Children.push_back(child);
            }
            if (!CEffectV2Runtime::Prewarm_Group(group, snapshot, targets.device, targets.context))
            { m_Status = "World Object effect prewarm failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
            m_EffectSnapshots.emplace(key, std::move(snapshot));
        }
    for (const auto& binding : instance.bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        if (!resource || resource->modelAssetId.empty())
        { m_Status = "World Object model binding is unavailable: " + binding.targetId; return false; }
        if (!targets.device || !targets.context)
        { m_Status = "World Object render device is unavailable."; return false; }
        auto model = m_ObjectModels.find(resource->objectId);
        if (model == m_ObjectModels.end())
        {
            OBJECT_MODEL staged;
            const auto path = CRuntimeAssetRoot::Resolve(resource->modelAssetId);
            if (path.empty())
            { m_Status = "World Object model path is invalid: " + resource->modelAssetId; return false; }
            staged.model = CModel::Create(targets.device, targets.context,
                resource->animated ? MODEL::ANIM : MODEL::NONANIM, path.string().c_str(),
                XMMatrixScaling(resource->modelPreScale, resource->modelPreScale, resource->modelPreScale));
            if (!staged.model || !staged.model->Get_NumMeshes())
            { m_Status = "World Object model admission failed: " + resource->modelAssetId; return false; }
            if (!resource->diffuseTextureAssetId.empty())
            {
                const auto texture = CRuntimeAssetRoot::Resolve(resource->diffuseTextureAssetId);
                if (texture.empty() || FAILED(DirectX::CreateDDSTextureFromFileEx(
                    targets.device.Get(), texture.c_str(), 0u, D3D11_USAGE_DEFAULT,
                    D3D11_BIND_SHADER_RESOURCE, 0u, 0u, DirectX::DDS_LOADER_FORCE_SRGB,
                    nullptr, &staged.diffuse)) || !staged.diffuse)
                { m_Status = "World Object texture admission failed: " + resource->diffuseTextureAssetId; return false; }
            }
            if (!staged.diffuse)
                for (uint32_t mesh = 0; mesh < staged.model->Get_NumMeshes(); ++mesh)
                    if (!staged.model->Has_MaterialTexture(mesh, aiTextureType_DIFFUSE))
                    { m_Status = "World Object diffuse texture is unavailable: " + resource->modelAssetId +
                        " (mesh " + std::to_string(mesh) + ")"; return false; }
            model = m_ObjectModels.emplace(resource->objectId, std::move(staged)).first;
        }
        if (sequence)
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId) continue;
                bool found = false;
                for (uint32_t i = 0; i < model->second.model->Get_NumAnimations(); ++i)
                    if (animation.clipName == model->second.model->Get_AnimationName(i)) found = true;
                if (!found) { m_Status = "World Object clip is absent: " + animation.clipName; return false; }
            }
    }
    return true;
}

bool_t CWorldSequencePlayer::Prepare_InstanceResources(const std::string& instanceId, const TARGET_SET& targets)
{
    const auto* instance = m_Document.Find_Instance(instanceId);
    if (!instance || !instance->enabled)
    { m_Status = "World Object state is absent or disabled: " + instanceId; return false; }
    if (!targets.Is_Complete())
    { m_Status = "World Object runtime targets are unavailable."; return false; }
    for (const auto& binding : instance->bindings)
    {
        if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        uint64_t targetId = 0;
        if (!Try_ParseTargetId(binding, targetId) ||
            (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
                !Find_Placement(*targets.pPlacements, targetId)) ||
            (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT &&
                !targets.pDeployRuntime->Find(targetId)))
        { m_Status = "World Object placement is outside the active map scope: " + binding.targetId; return false; }
    }
    return Prepare_ObjectMotionChain(*instance, targets);
}

void CWorldSequencePlayer::Release_Objects(ACTIVE_INSTANCE& active)
{
    for (const auto& effect : active.effects) CEffectV2Runtime::Stop_Group(effect.handle);
    active.effects.clear();
    active.emissionAnchors.clear();
    for (auto& entry : active.objects)
        if (entry.object)
        {
            entry.object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(entry.levelIndex,
                CWorldSequenceObject::LAYER_TAG, entry.object);
        }
    active.objects.clear();
}

bool_t CWorldSequencePlayer::Try_GetObjectPivot(const std::string& instanceId, float4x4_t& out,
    const uint32_t emissionIndex) const
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end()) return false;
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = nullptr != instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (nullptr == sequence || sequence->objectMotion.emissions.empty())
    {
        if (0u != emissionIndex || active->objects.size() != 1 || !active->objects.front().object ||
            !active->objects.front().object->Is_Visible()) return false;
        out = active->objects.front().object->Get_SampledWorld();
        return true;
    }
    // An authored row is one clone per anchor; a WORLD motion has exactly one anchor.
    const CWorldSequenceObject* found = nullptr;
    for (const auto& entry : active->objects)
    {
        if (entry.emissionIndex != emissionIndex || !entry.object) continue;
        if (found) return false;
        found = entry.object.get();
    }
    if (!found || !found->Is_Visible()) return false;
    out = found->Get_SampledWorld();
    return true;
}

std::string CWorldSequencePlayer::Get_ObjectSampleStatus(const std::string& instanceId) const
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end()) return "World Object preview is not active.";
    if (!active->objectSampleStatus.empty()) return active->objectSampleStatus;
    const auto* instance = m_Document.Find_Instance(instanceId);
    if (!instance) return "World Object preview instance is unavailable.";
    const bool hasObjects = std::any_of(instance->bindings.begin(), instance->bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
    if (!hasObjects) return "Placed World Object state sampled at its authored map placement.";
    size_t visible = 0;
    const CWorldSequenceObject* first = nullptr;
    for (const auto& entry : active->objects)
        if (entry.object && entry.object->Is_Visible())
        { ++visible; if (!first) first = entry.object.get(); }
    if (!first) return "World Object: 0 visible (outside lifetime or hidden by the current key).";
    const auto& world = first->Get_SampledWorld();
    std::ostringstream status;
    status << "World Object: " << visible << " visible, first position (" << std::fixed << std::setprecision(2)
        << world._41 << ", " << world._42 << ", " << world._43 << ").";
    return status.str();
}

bool_t CWorldSequencePlayer::Get_EmissionAnchor(ACTIVE_INSTANCE& active, const TARGET_SET& targets,
    const std::string& key, const f32_t birthMs, const PLAYER_ANCHOR& baseline, PLAYER_ANCHOR& out)
{
    out = baseline;
    if (!targets.objectEmissionAnchor) return true;
    const auto existing = active.emissionAnchors.find(key);
    if (existing != active.emissionAnchors.end()) { out = existing->second; return true; }
    if (!std::isfinite(birthMs) || birthMs < 0.f || !targets.objectEmissionAnchor(birthMs, out.world))
    { m_Status = "World Object emission origin is unavailable at " + std::to_string(birthMs) + " ms."; return false; }
    const auto* values = reinterpret_cast<const f32_t*>(&out.world);
    for (size_t i = 0; i < 16u; ++i)
        if (!std::isfinite(values[i]))
        { m_Status = "World Object emission origin is not finite."; return false; }
    const matrix_t world = XMLoadFloat4x4(&out.world);
    if (std::abs(out.world._14) > .00001f || std::abs(out.world._24) > .00001f ||
        std::abs(out.world._34) > .00001f || std::abs(out.world._44 - 1.f) > .00001f ||
        std::abs(XMVectorGetX(XMMatrixDeterminant(world))) < .000001f)
    { m_Status = "World Object emission origin is not an invertible affine transform."; return false; }
    out.emissionOverride = true;
    active.emissionAnchors.emplace(key, out);
    return true;
}

bool_t CWorldSequencePlayer::Sample_ObjectWorld(const ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& slotId,
    const PLAYER_ANCHOR& anchor, const uint32_t emitter, const f32_t ageMs, float4x4_t& out)
{
    const auto& motion = sequence.objectMotion;
    const auto* track = Find_Track(sequence, slotId);
    const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
    const float seconds = ageMs * 0.001f;
    uint32_t random = motion.seed ^ ((emitter + 1u) * 0x9e3779b9u);
    const auto randomUnit = [&random]()
    { random ^= random << 13; random ^= random >> 17; random ^= random << 5;
        return static_cast<float>(random & 0xffffffu) / 16777215.f; };
    const float spread = XMConvertToRadians(motion.spreadDegrees);
    const matrix_t direction = sequence.effectTracks.empty() ?
        XMMatrixRotationRollPitchYaw((randomUnit() - .5f) * spread, (randomUnit() - .5f) * spread * 2.f, 0.f) :
        XMMatrixRotationY((randomUnit() - .5f) * spread);
    const vector_t velocity = XMVector3TransformNormal(XMLoadFloat3(&motion.velocity), direction);
    const matrix_t revolution = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(motion.revolutionDegreesPerSecond.x * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.y * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.z * seconds));
    const vector_t orbit = XMLoadFloat3(&motion.revolutionOffset);
    const vector_t position = XMLoadFloat3(&key.positionOffset) +
        velocity * seconds + XMLoadFloat3(&motion.acceleration) * (.5f * seconds * seconds) +
        XMVector3TransformNormal(orbit, revolution) - orbit;
    const vector_t scale = XMLoadFloat3(&resource.scale) * XMLoadFloat3(&key.scaleMultiplier);
    matrix_t basis = XMLoadFloat4x4(&anchor.world);
    for (int axis = 0; axis < 3; ++axis)
    {
        const float length = XMVectorGetX(XMVector3LengthSq(basis.r[axis]));
        if (!std::isfinite(length) || length < 1e-6f)
        { m_Status = "World Object anchor transform is invalid: " + resource.objectId; return false; }
        basis.r[axis] = XMVectorSetW(XMVector3Normalize(basis.r[axis]), 0.f);
    }
    const matrix_t rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(motion.angularVelocityDegrees.x * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.y * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.z * seconds));
    matrix_t world = XMMatrixScalingFromVector(scale) * rotation * XMMatrixTranslationFromVector(position);
    /* An authored row turns the whole local motion, orbit included, so one row
       set fans a path into lanes or lays a ring of copies orbiting one centre. */
    if (!motion.emissions.empty())
    {
        const auto& emission = motion.emissions[(std::min)(static_cast<size_t>(emitter), motion.emissions.size() - 1u)];
        world *= XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&emission.positionOffset));
    }
    if (!(active.placement || anchor.emissionOverride))
        world *= XMMatrixTranslationFromVector(XMLoadFloat3(&instance.position));
    if (!anchor.emissionOverride) world *= basis;
    if (active.placement)
    {
        const auto& placement = *active.placement;
        world *= XMMatrixScalingFromVector(XMLoadFloat3(&placement.scale)) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(placement.rotationDegrees.x),
                XMConvertToRadians(placement.rotationDegrees.y), XMConvertToRadians(placement.rotationDegrees.z)) *
            (anchor.emissionOverride ? XMMatrixIdentity() : XMMatrixTranslationFromVector(XMLoadFloat3(&placement.position)));
    }
    else if (!anchor.emissionOverride) world.r[3] += XMVectorSet(active.positionOffset.x, active.positionOffset.y, active.positionOffset.z, 0.f);
    if (anchor.emissionOverride) world *= basis;
    XMStoreFloat4x4(&out, world);
    return true;
}

bool_t CWorldSequencePlayer::Apply_Objects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const TARGET_SET& targets, const f32_t localMs, const bool_t visible, const bool_t holdFinalPose,
    const f32_t emissionStartMs, const f32_t emissionRate, const std::string& emissionMotionId)
{
    active.objectSampleStatus.clear();
    for (auto& entry : active.objects) entry.object->Hide();
    if (!visible || std::none_of(instance.bindings.begin(), instance.bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; })) return true;
    std::vector<PLAYER_ANCHOR> anchors;
    if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
    {
        if (targets.playerAnchors) anchors = targets.playerAnchors();
        if (anchors.empty())
        {
            active.objectSampleStatus = "World Object Character anchor is waiting for a living replicated player.";
            return true;
        }
    }
    else
    {
        PLAYER_ANCHOR world;
        XMStoreFloat4x4(&world.world, XMMatrixIdentity());
        anchors.push_back(world);
    }
    const auto& motion = sequence.objectMotion;
    for (const auto& binding : instance.bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        const auto model = m_ObjectModels.find(binding.targetId);
        const auto* track = Find_Track(sequence, binding.slotId);
        if (!resource || model == m_ObjectModels.end())
        { m_Status = "World Object model was not prepared: " + binding.targetId; return false; }
        for (const auto& anchor : anchors)
            for (uint32_t emitter = 0; emitter < motion.EmissionCount(); ++emitter)
            {
                const f32_t delayMs = static_cast<f32_t>(motion.EmissionDelayMs(emitter));
                const f32_t ageMs = localMs - delayMs;
                if (ageMs < 0.f || (!holdFinalPose && !sequence.effectTracks.empty() && ageMs >= sequence.durationMs)) continue;
                const f32_t birthMs = emissionStartMs + delayMs / emissionRate;
                if (!sequence.effectTracks.empty() && active.durationMs && birthMs >= active.durationMs) continue;
                PLAYER_ANCHOR emissionAnchor;
                const auto emissionKey = emissionMotionId + ":" + std::to_string(emissionStartMs) + ":" + std::to_string(emitter);
                if (!Get_EmissionAnchor(active, targets, emissionKey, birthMs, anchor, emissionAnchor)) return false;
                auto found = std::find_if(active.objects.begin(), active.objects.end(), [&](const auto& value)
                { return value.slotId == binding.slotId && value.entityId == anchor.entityId && value.emissionIndex == emitter; });
                if (found == active.objects.end())
                {
                    size_t total = 0;
                    for (const auto& value : m_Active) total += value.objects.size();
                    if (total >= 1024u) { m_Status = "World Object instance budget reached (1024)."; return false; }
                    CWorldSequenceObject::DESC desc;
                    desc.levelIndex = targets.levelIndex;
                    desc.modelPrototype = model->second.model;
                    desc.diffuseTexture = model->second.diffuse;
                    shared_ptr<CGameObject> staged;
                    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
                        CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex,
                        CWorldSequenceObject::LAYER_TAG, &desc, &staged)))
                    { m_Status = "World Object clone/shader creation failed: " + resource->objectId; return false; }
                    active.objects.push_back({binding.slotId, anchor.entityId, emitter, targets.levelIndex,
                        dynamic_pointer_cast<CWorldSequenceObject>(staged)});
                    found = active.objects.end() - 1;
                }
                const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
                float4x4_t stored;
                if (!Sample_ObjectWorld(active, instance, sequence, *resource, binding.slotId, emissionAnchor, emitter,
                    (std::min)(ageMs, static_cast<f32_t>(sequence.durationMs)), stored)) return false;
                f32_t windowEnd = 0.f;
                const auto* animation = Find_AnimationTrackAt(sequence, binding.slotId, ageMs, windowEnd);
                if (!found->object)
                { m_Status = "World Object clone type is invalid: " + resource->objectId; return false; }
                if (!found->object->Get_RenderStatus().empty())
                { m_Status = found->object->Get_RenderStatus() + " / " + resource->objectId; return false; }
                if (!found->object->Sample(stored, key.visible, animation, ageMs, windowEnd))
                { m_Status = "World Object transform/animation sample failed: " + resource->objectId; return false; }
            }
    }
    return true;
}

bool_t CWorldSequencePlayer::Apply_ObjectEffects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    std::vector<PLAYER_ANCHOR> anchors;
    if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
    {
        if (targets.playerAnchors) anchors = targets.playerAnchors();
    }
    else
    {
        PLAYER_ANCHOR anchor;
        XMStoreFloat4x4(&anchor.world, XMMatrixIdentity());
        anchors.push_back(anchor);
    }
    std::unordered_set<std::string> wanted;
    const auto& binding = instance.bindings.front();
    const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
    if (!resource) return false;
    // Resolve every still-visible event from the owning clock. This preserves an
    // earlier NEXT/LOOP tail and makes a direct seek equivalent to ordinary play.
    const auto sampleChain = [&](const std::string& firstId, const f32_t firstStartMs,
        const f32_t cutoffMs) -> bool_t
    {
        const auto* motion = m_Document.Find_Instance(firstId);
        f32_t start = firstStartMs;
        for (uint32_t depth = 0; motion && depth <= 32u; ++depth)
        {
            const auto* sequence = m_Document.Find_Template(motion->templateId);
            if (!sequence) return false;
            const f32_t rate = motion->playbackSpeed * active.playbackSpeed;
            start += motion->startDelayMs;
            const f32_t localMs = (active.elapsedMs - start) * rate;
            if (localMs < 0.f || start > cutoffMs) break;
            const f32_t period = static_cast<f32_t>(sequence->ObjectSpanMs());
            for (const auto& effect : sequence->effectTracks)
            {
                const auto snapshot = m_EffectSnapshots.find(effect.resourceKind + ":" + effect.resourceId);
                if (snapshot == m_EffectSnapshots.end())
                { m_Status = "World Object effect was not prepared: " + effect.resourceId; return false; }
                const f32_t trigger = static_cast<f32_t>(sequence->EffectStartMs(effect));
                for (uint32_t emitter = 0; emitter < sequence->objectMotion.EmissionCount(); ++emitter)
                {
                    const f32_t birth = static_cast<f32_t>(sequence->objectMotion.EmissionDelayMs(emitter)) + trigger;
                    if (localMs < birth) continue;
                    const bool loop = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP;
                    const uint64_t last = loop ? static_cast<uint64_t>(std::floor((localMs - birth) / period)) : 0u;
                    const uint64_t first = loop ? static_cast<uint64_t>((std::max)(0.0,
                        std::floor((localMs - birth - effect.durationMs) / period) + 1.0)) : 0u;
                    if (last < first) continue;
                    if (last - first > 1024u)
                    { m_Status = "World Object Effect overlap exceeds 1024 occurrences; increase Motion Lifetime."; return false; }
                    for (uint64_t epoch = first; epoch <= last; ++epoch)
                    {
                        const f32_t eventLocal = birth + static_cast<f32_t>(epoch) * period;
                        const f32_t ageMs = localMs - eventLocal;
                        const f32_t birthMs = start + (eventLocal - trigger) / rate;
                        if (ageMs < 0.f || ageMs >= effect.durationMs || birthMs >= cutoffMs) continue;
                        for (const auto& anchor : anchors)
                        {
                            const auto key = firstId + ":" + motion->instanceId + ":" + std::to_string(firstStartMs) + ":" +
                                effect.effectTrackId + ":" + std::to_string(epoch) + ":" + std::to_string(emitter) + ":" + std::to_string(anchor.entityId);
                            wanted.insert(key);
                            auto found = std::find_if(active.effects.begin(), active.effects.end(),
                                [&](const auto& value) { return value.key == key; });
                            if (found == active.effects.end())
                            {
                                size_t total = 0;
                                for (const auto& value : m_Active) total += value.effects.size();
                                if (total >= 1024u)
                                { m_Status = "World Object Effect occurrence budget reached (1024)."; return false; }
                                PLAYER_ANCHOR emissionAnchor;
                                const f32_t emissionStartMs = start + static_cast<f32_t>(epoch) * period / rate;
                                const auto emissionKey = motion->instanceId + ":" + std::to_string(emissionStartMs) + ":" + std::to_string(emitter);
                                if (!Get_EmissionAnchor(active, targets, emissionKey, birthMs, anchor, emissionAnchor)) return false;
                                float4x4_t objectWorld;
                                if (!Sample_ObjectWorld(active, instance, *sequence, *resource, effect.slotId,
                                    emissionAnchor, emitter, trigger, objectWorld)) return false;
                                matrix_t pivot = XMLoadFloat4x4(&objectWorld);
                                // Object mesh scale is independent of the authored Effect's metre scale.
                                for (int axis = 0; axis < 3; ++axis)
                                    pivot.r[axis] = XMVectorSetW(XMVector3Normalize(pivot.r[axis]), 0.f);
                                const matrix_t local = XMMatrixScalingFromVector(XMLoadFloat3(&effect.scale)) *
                                    XMMatrixRotationRollPitchYaw(XMConvertToRadians(effect.rotationDegrees.x),
                                        XMConvertToRadians(effect.rotationDegrees.y), XMConvertToRadians(effect.rotationDegrees.z)) *
                                    XMMatrixTranslationFromVector(XMLoadFloat3(&effect.positionOffset));
                                EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                                XMStoreFloat4x4(&playback.PivotWorld, local * pivot);
                                playback.bExternalClock = true;
                                playback.bProductOwned = true;
                                // Sample_Group receives authored seconds; the caller already applied Motion speed.
                                playback.fDurationSeconds = -1.f;
                                const uint32_t handle = effect.resourceKind == "GROUP" ?
                                    CEffectV2Runtime::Play_Group(*snapshot->second->Find_Group(effect.resourceId), snapshot->second,
                                        playback, targets.device, targets.context) :
                                    CEffectV2Runtime::Play_Leaf(effect.resourceId, snapshot->second, playback, targets.device, targets.context);
                                if (!handle)
                                { m_Status = "World Object effect play failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                                active.effects.push_back({key, handle});
                                found = active.effects.end() - 1;
                            }
                            if (!CEffectV2Runtime::Sample_Group(found->handle, ageMs * .001f, true, targets.device, targets.context))
                            { m_Status = "World Object effect sample failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                        }
                    }
                }
            }
            if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
            start += period / rate;
            motion = m_Document.Find_Instance(motion->nextMotionId);
        }
        return true;
    };
    const f32_t end = active.durationMs ? static_cast<f32_t>(active.durationMs) : (std::numeric_limits<f32_t>::max)();
    const bool changed = !active.motionInstanceId.empty() && active.elapsedMs >= active.motionStartMs;
    if (!sampleChain(active.instanceId, 0.f, changed ? (std::min)(end, active.motionStartMs) : end) ||
        (changed && !sampleChain(active.motionInstanceId, active.motionStartMs, end))) return false;
    for (size_t index = 0; index < active.effects.size();)
    {
        if (wanted.contains(active.effects[index].key)) { ++index; continue; }
        CEffectV2Runtime::Stop_Group(active.effects[index].handle);
        active.effects.erase(active.effects.begin() + static_cast<ptrdiff_t>(index));
    }
    return true;
}

bool_t Client::CWorldSequencePlayer::Try_GetSequencePivot(const std::string& instanceId, float4x4_t& out,
 const uint32_t emissionIndex) const
{
 if (Try_GetObjectPivot(instanceId, out, emissionIndex)) return true;
 // Placed map aliases have no emission rows; only row 0 can name them.
 if (0u != emissionIndex) return false;
 const auto* instance = Get_Document().Find_Instance(instanceId);
 if (!instance) return false;
 const WORLD_SEQUENCE_BINDING* binding = nullptr;
 for (const auto& candidate : instance->bindings)
 {
  if (candidate.targetKind != WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT) continue;
  if (candidate.slotId == "object") { binding = &candidate; break; }
  if (binding) return false;
  binding = &candidate;
 }
 if (!binding) return false;
 uint64_t placementId = 0;
 MAP_PLACEMENT_RECORD record;
 if (!CWorldSequencePlayer::Try_ParseTargetId(*binding, placementId) ||
  !Try_GetSampledPlacementRecord(instanceId, placementId, record)) return false;
 XMStoreFloat4x4(&out, XMMatrixScaling(record.signedScale.x, record.signedScale.y, record.signedScale.z) *
  XMMatrixRotationQuaternion(XMLoadFloat4(&record.rotationQuaternion)) *
  XMMatrixTranslation(record.position.x, record.position.y, record.position.z));
 return true;
}
