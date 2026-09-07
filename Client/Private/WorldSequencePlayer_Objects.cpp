#include "WorldSequencePlayer.h"
#include "WorldSequenceObject.h"
#include "DeployPropObject.h"
#include "GameInstance.h"
#include "Model.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "RuntimeAssetRoot.h"
#include <algorithm>
#include <cmath>

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
    m_Document = document;
    m_Status = status = "World Object document admitted.";
    return true;
}

bool_t CWorldSequencePlayer::Prepare_ObjectResources(
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    const auto* sequence = m_Document.Find_Template(instance.templateId);
    for (const auto& binding : instance.bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        if (!resource || resource->modelAssetId.empty() || !targets.device || !targets.context) return false;
        auto model = m_ObjectModels.find(resource->objectId);
        if (model == m_ObjectModels.end())
        {
            OBJECT_MODEL staged;
            const auto path = CRuntimeAssetRoot::Resolve(resource->modelAssetId);
            if (path.empty()) return false;
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
    return Prepare_ObjectResources(*instance, targets);
}

void CWorldSequencePlayer::Release_Objects(ACTIVE_INSTANCE& active)
{
    for (auto& entry : active.objects)
        if (entry.object)
        {
            entry.object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(entry.levelIndex,
                CWorldSequenceObject::LAYER_TAG, entry.object);
        }
    active.objects.clear();
}

bool_t CWorldSequencePlayer::Try_GetObjectPivot(const std::string& instanceId, float4x4_t& out) const
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end() || active->objects.size() != 1 || !active->objects.front().object ||
        !active->objects.front().object->Is_Visible()) return false;
    out = active->objects.front().object->Get_SampledWorld();
    return true;
}

bool_t CWorldSequencePlayer::Apply_Objects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const TARGET_SET& targets, const f32_t localMs, const bool_t visible)
{
    for (auto& entry : active.objects) entry.object->Hide();
    if (!visible) return true;
    std::vector<PLAYER_ANCHOR> anchors;
    if (instance.anchorKind == "PLAYER")
    {
        if (targets.playerAnchors) anchors = targets.playerAnchors();
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
        if (!resource || model == m_ObjectModels.end()) return false;
        for (const auto& anchor : anchors)
            for (uint32_t emitter = 0; emitter < motion.count; ++emitter)
            {
                const f32_t ageMs = localMs - static_cast<f32_t>(emitter) * motion.intervalMs;
                if (ageMs < 0.f) continue;
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
                        CWorldSequenceObject::LAYER_TAG, &desc, &staged))) return false;
                    active.objects.push_back({binding.slotId, anchor.entityId, emitter, targets.levelIndex,
                        dynamic_pointer_cast<CWorldSequenceObject>(staged)});
                    found = active.objects.end() - 1;
                }
                const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
                const float seconds = ageMs * 0.001f;
                uint32_t random = motion.seed ^ ((emitter + 1u) * 0x9e3779b9u);
                const auto randomUnit = [&random]()
                { random ^= random << 13; random ^= random >> 17; random ^= random << 5;
                    return static_cast<float>(random & 0xffffffu) / 16777215.f; };
                const float spread = XMConvertToRadians(motion.spreadDegrees);
                const matrix_t direction = XMMatrixRotationRollPitchYaw((randomUnit() - .5f) * spread,
                    (randomUnit() - .5f) * spread * 2.f, 0.f);
                const vector_t velocity = XMVector3TransformNormal(XMLoadFloat3(&motion.velocity), direction);
                const matrix_t revolution = XMMatrixRotationRollPitchYaw(
                    XMConvertToRadians(motion.revolutionDegreesPerSecond.x * seconds),
                    XMConvertToRadians(motion.revolutionDegreesPerSecond.y * seconds),
                    XMConvertToRadians(motion.revolutionDegreesPerSecond.z * seconds));
                const vector_t orbit = XMLoadFloat3(&motion.revolutionOffset);
                const vector_t position = XMLoadFloat3(&instance.position) + XMLoadFloat3(&key.positionOffset) +
                    velocity * seconds + XMLoadFloat3(&motion.acceleration) * (.5f * seconds * seconds) +
                    XMVector3TransformNormal(orbit, revolution) - orbit;
                const vector_t scale = XMLoadFloat3(&resource->scale) * XMLoadFloat3(&key.scaleMultiplier);
                matrix_t basis = XMLoadFloat4x4(&anchor.world);
                for (int axis = 0; axis < 3; ++axis)
                {
                    const float length = XMVectorGetX(XMVector3LengthSq(basis.r[axis]));
                    if (!std::isfinite(length) || length < 1e-6f) return false;
                    basis.r[axis] = XMVectorSetW(XMVector3Normalize(basis.r[axis]), 0.f);
                }
                const matrix_t rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) *
                    XMMatrixRotationRollPitchYaw(XMConvertToRadians(motion.angularVelocityDegrees.x * seconds),
                        XMConvertToRadians(motion.angularVelocityDegrees.y * seconds),
                        XMConvertToRadians(motion.angularVelocityDegrees.z * seconds));
                matrix_t world = XMMatrixScalingFromVector(scale) * rotation * XMMatrixTranslationFromVector(position) * basis;
                world.r[3] += XMVectorSet(active.positionOffset.x, active.positionOffset.y, active.positionOffset.z, 0.f);
                float4x4_t stored;
                XMStoreFloat4x4(&stored, world);
                f32_t windowEnd = 0.f;
                const auto* animation = Find_AnimationTrackAt(sequence, binding.slotId, ageMs, windowEnd);
                if (!found->object || !found->object->Sample(stored, key.visible, animation, ageMs, windowEnd)) return false;
            }
    }
    return true;
}
