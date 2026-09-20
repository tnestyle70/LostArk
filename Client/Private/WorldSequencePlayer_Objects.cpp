#include "WorldSequencePlayer.h"
#include "ActorCatalog.h"
#include "WorldSequenceObject.h"
#include "DeployPropObject.h"
#include "GameInstance.h"
#include "Model.h"
#include "NpcPresentationAssetService.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "RuntimeAssetRoot.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "Effect_PresentationService.h"
#include "Effect_Playback.h"
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace Client;
using namespace Engine;

CWorldSequencePlayer::~CWorldSequencePlayer() { Clear(); }

namespace
{
/* The product Valtan part group: a static weapon on the body's grip bone and
   skinned armour plates on its palette, as CValtan builds them. */
void Fill_PresentationParts(const std::string& archetypeId, CWorldSequenceObject::DESC& desc)
{
    desc.presentationParts.clear();
    desc.materialProfileId.clear();
    const BOSS_ACTOR_ENTRY* actor = archetypeId.empty() ? nullptr : CActorCatalog::Find_Boss(archetypeId);
    if (!actor) return;
    desc.materialProfileId = "material.valtan.monster-base.v1";
    desc.presentationParts.push_back({ CValtanPresentationAssetService::Get_WeaponModelPrototypeTag(archetypeId),
        L"Prototype_Component_Shader_VtxMeshBinary", CValtan::WEAPON_SOCKET_BONE });
    for (const BOSS_ARMOR_PART_ENTRY& armor : actor->armorParts)
        desc.presentationParts.push_back({ CValtan::Build_ArmorModelPrototypeTag(armor.stateMask, archetypeId),
            L"Prototype_Component_Shader_VtxAnimMeshBinary", std::string() });
}

std::string Narrow_PrototypeTag(const wstring_t& tag)
{
    std::string text;
    text.reserve(tag.size());
    for (const wchar_t character : tag) text.push_back(character < 128 ? static_cast<char>(character) : '?');
    return text;
}

#ifdef _DEBUG
bool Sample_ObjectCollider(const WORLD_SEQUENCE_COLLIDER_TRACK& collider,
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const WORLD_SEQUENCE_TRANSFORM_KEY& key,
    const WORLD_SEQUENCE_OBJECT_MOTION& motion, const uint32_t emitter,
    const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement,
    const CWorldSequenceObject& object, CWorldSequencePlayer::OBJECT_COLLIDER_SAMPLE& out)
{
    const float emissionYaw = motion.emissions.empty() ? 0.f : motion.emissions[emitter].yawDegrees;
    const vector_t localScale = XMLoadFloat3(&resource.scale) * XMLoadFloat3(&key.scaleMultiplier);
    const vector_t placementScale = placement ? XMLoadFloat3(&placement->scale) : XMVectorReplicate(1.f);
    const float placementYaw = placement ? placement->rotationDegrees.y : 0.f;
    const matrix_t groundBasis = XMMatrixScalingFromVector(localScale) *
        XMMatrixRotationY(XMConvertToRadians(emissionYaw)) * XMMatrixScalingFromVector(placementScale) *
        XMMatrixRotationY(XMConvertToRadians(placementYaw));
    out.behavior = collider.behavior;
    out.yawDegrees = emissionYaw + placementYaw + collider.yawDegrees;
    XMStoreFloat3(&out.halfExtents, XMVectorAbs(XMLoadFloat3(&collider.halfExtents) * localScale * placementScale));
    out.hasGrip = collider.behavior == "HOOK_CAPTURE";
    if (out.hasGrip)
    {
        float4x4_t attachment;
        if (!object.Try_GetAttachmentWorld(collider.attachmentBone, attachment)) return false;
        const matrix_t world = XMLoadFloat4x4(&attachment);
        XMStoreFloat3(&out.center, XMVector3TransformCoord(XMLoadFloat3(&collider.positionOffset), world));
        XMStoreFloat3(&out.gripPosition, XMVector3TransformCoord(XMLoadFloat3(&collider.gripLocalOffset), world));
    }
    else
    {
        const matrix_t world = XMLoadFloat4x4(&object.Get_SampledWorld());
        XMStoreFloat3(&out.center, world.r[3] +
            XMVector3TransformNormal(XMLoadFloat3(&collider.positionOffset), groundBasis));
    }
    return true;
}
#endif

// Use the same clip windows, ticks and end policy as WorldSequenceObject::Sample,
// but sample the immutable CModel skeleton without changing the visible palette.
bool Sample_ObjectEffectBone(const std::shared_ptr<CModel>& model,
    const WORLD_SEQUENCE_TEMPLATE& sequence, const std::string& slotId,
    const std::string& bone, const float sampleMs, float4x4_t& out, std::string& error)
{
    if (!model || !model->Has_Bone(bone.c_str()))
    { error = "World Object Effect bone is unavailable: " + bone; return false; }
    f32_t windowEnd = 0.f;
    const auto* animation = CWorldSequencePlayer::Find_AnimationTrackAt(sequence, slotId, sampleMs, windowEnd);
    if (!animation) { XMStoreFloat4x4(&out, model->Get_BoneMatrix(bone.c_str())); return true; }
    uint32_t index = UINT32_MAX;
    for (uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
        if (animation->clipName == model->Get_AnimationName(i)) { index = i; break; }
    float position = 0.f, duration = 0.f;
    if (index == UINT32_MAX || !model->Get_AnimationProgress(index, position, duration) || duration <= 0.f)
    { error = "World Object Effect animation is unavailable: " + animation->clipName; return false; }
    float ticks = 0.f;
    if (!CWorldSequenceDocument::Try_SampleAnimationTicks(*animation, sampleMs, windowEnd,
        model->Get_AnimationTickPerSecond(index), duration, ticks))
    { error = "World Object Effect animation source range is invalid: " + animation->clipName; return false; }
    const uint32_t boneIndex = static_cast<uint32_t>(model->Find_BoneIndex(bone.c_str()));
    if (!model->Sample_AnimationBoneCombinedMatrices(animation->clipName.c_str(), ticks,
        std::span<const uint32_t>(&boneIndex, 1u), std::span<float4x4_t>(&out, 1u)))
    { error = "World Object Effect bone sample failed: " + bone; return false; }
    return true;
}

bool Sample_ObjectEffectAttachments(const EFFECT_DOCUMENT_DESC& document,
    const std::shared_ptr<CModel>& model, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const std::string& slotId, const float sampleMs, const float4x4_t& root,
    std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
{
    std::unordered_map<std::string, float4x4_t> bones;
    for (const auto& element : document.Elements)
    {
        const auto& attachment = element.ActionCueAttachment;
        if (!element.bVisible || !attachment.bEnabled || !attachment.bFollow) continue;
        if (attachment.strRuntimeAnchorSlotId.empty())
        { error = "World Object Effect source attachment has no stable slot."; return false; }
        matrix_t anchor;
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
        {
            const auto* camera = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
            if (!camera) { error = "World Object Effect camera anchor is unavailable."; return false; }
            anchor = XMLoadFloat4x4(camera);
        }
        else
        {
            auto found = bones.find(attachment.strRuntimeBoneName);
            if (found == bones.end())
            {
                float4x4_t bone;
                // A static prop has no source-character skeleton (b_root, FX_* sockets).
                // Mirror the owner-anchored product path, which skips a missing bone,
                // but keep the slot present: transform-history sampling requires every
                // follow slot, so the attachment rides the object pivot instead.
                // The explicit effect-track bone (effect.bone) stays strict in the provider.
                if (model && !model->Has_Bone(attachment.strRuntimeBoneName.c_str()))
                    XMStoreFloat4x4(&bone, XMMatrixIdentity());
                else if (!Sample_ObjectEffectBone(model, sequence, slotId, attachment.strRuntimeBoneName,
                    sampleMs, bone, error)) return false;
                found = bones.emplace(attachment.strRuntimeBoneName, bone).first;
            }
            if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::BONE)
                anchor = XMLoadFloat4x4(&found->second) * XMLoadFloat4x4(&root);
            else if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
            {
                float4x4_t yawAnchor;
                if (!CEffectPlayback::Build_OwnerYawBoneAnchorWorld(found->second, root, root, yawAnchor))
                { error = "World Object Effect owner-yaw anchor is invalid."; return false; }
                anchor = XMLoadFloat4x4(&yawAnchor);
            }
            else { error = "World Object Effect source attachment orientation is unsupported."; return false; }
        }
        const auto& socket = attachment.SocketLocalTransform;
        const matrix_t world = XMMatrixScalingFromVector(XMLoadFloat3(&socket.vScale)) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(socket.vRotationDegrees.x),
                XMConvertToRadians(socket.vRotationDegrees.y), XMConvertToRadians(socket.vRotationDegrees.z)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&socket.vPosition)) * anchor;
        const float determinant = XMVectorGetX(XMMatrixDeterminant(world));
        if (XMMatrixIsNaN(world) || XMMatrixIsInfinite(world) || !std::isfinite(determinant) || std::abs(determinant) < 1.e-12f)
        { error = "World Object Effect source attachment is singular or non-finite."; return false; }
        float4x4_t value; XMStoreFloat4x4(&value, world);
        const auto [found, inserted] = anchors.emplace(attachment.strRuntimeAnchorSlotId, value);
        if (!inserted)
            for (size_t r = 0; r < 4; ++r) for (size_t c = 0; c < 4; ++c)
                if (std::abs(found->second.m[r][c] - value.m[r][c]) > .0001f)
                { error = "World Object Effect source slot has conflicting bones."; return false; }
    }
    return true;
}
}


bool_t CWorldSequencePlayer::Resolve_BossBoneAnchor(const std::shared_ptr<CModel>& model,
    const float4x4_t& root, const std::string& bone, PLAYER_ANCHOR& out, std::string& status)
{
    if (!model || (!bone.empty() && !model->Has_Bone(bone.c_str())))
    { status = "World Object boss BODY bone is unavailable: " + bone; return false; }
    const matrix_t basis = bone.empty() ? XMLoadFloat4x4(&root) :
        model->Get_BoneMatrix(bone.c_str()) * XMLoadFloat4x4(&root);
    XMStoreFloat4x4(&out.world, basis);
    const auto* values = reinterpret_cast<const f32_t*>(&out.world);
    for (size_t i = 0u; i < 16u; ++i)
        if (!std::isfinite(values[i]))
        { status = "World Object boss BODY bone pose is not finite: " + bone; return false; }
    if (std::abs(XMVectorGetX(XMMatrixDeterminant(basis))) < .000001f)
    { status = "World Object boss BODY bone pose is singular: " + bone; return false; }
    out.bodyModel = model;
    // The object sampler preserves the socket translation and normalizes its
    // axes. Import scale belongs to the prop; boss scale is already in this pose.
    return true;
}

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
    const bool_t admitted = Set_DocumentBatch(document, targets, {this}, status);
    m_Status = status;
    return admitted;
}

bool_t CWorldSequencePlayer::Set_DocumentBatch(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, const std::vector<CWorldSequencePlayer*>& players, std::string& status)
{
    if (!targets.Is_Complete())
    { status = "World Object runtime targets are unavailable."; return false; }
    std::unordered_set<CWorldSequencePlayer*> unique;
    if (players.empty() || std::any_of(players.begin(), players.end(), [&](auto* player) {
        return !player || !unique.insert(player).second; }))
    { status = "World Object document batch has an empty or duplicate player."; return false; }
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    Collect_ValidationTargets(targets, placements, deploy);
    if (!document.Validate(placements, deploy, status))
        return false;
    std::vector<CWorldSequenceDocument> staged;
    staged.reserve(players.size());
    for (size_t i = 0; i < players.size(); ++i) staged.push_back(document);
    for (size_t i = 0; i < players.size(); ++i)
    {
        auto& player = *players[i];
        player.Stop_All(targets, true);
        player.Clear_PreparedObjects();
        player.m_ObjectModels.clear();
        player.m_EffectSnapshots.clear();
        player.m_Document = std::move(staged[i]);
        player.m_Status = "World Object document admitted.";
    }
    status = "World Object document admitted.";
    return true;
}

bool_t CWorldSequencePlayer::Replace_DocumentKeepingModels(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, std::string& status)
{
    if (!targets.Is_Complete())
    {
        status = "World Object runtime targets are unavailable.";
        m_Status = status;
        return false;
    }
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    Collect_ValidationTargets(targets, placements, deploy);
    if (!document.Validate(placements, deploy, status))
    {
        m_Status = status;
        return false;
    }
    CWorldSequenceDocument staged = document;
    Stop_All(targets, true);
    Clear_PreparedObjects();
    // A kept model must still be what its resource asks for. Anything renamed,
    // removed or changed in its model inputs is rebuilt on the next Play.
    size_t kept = 0u;
    for (auto entry = m_ObjectModels.begin(); entry != m_ObjectModels.end();)
    {
        const auto* before = m_Document.Find_ObjectResource(entry->first);
        const auto* after = staged.Find_ObjectResource(entry->first);
        if (nullptr == before || nullptr == after || !Same_ObjectModelInputs(*before, *after))
            entry = m_ObjectModels.erase(entry);
        else
        {
            ++kept;
            ++entry;
        }
    }
    m_EffectSnapshots.clear();
    m_Document = std::move(staged);
    status = "World Object document admitted; kept " + std::to_string(kept) + " prepared model(s).";
    m_Status = status;
    return true;
}

bool_t CWorldSequencePlayer::Same_ObjectModelInputs(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& left, const WORLD_SEQUENCE_OBJECT_RESOURCE& right)
{
    return left.modelAssetId == right.modelAssetId && left.modelPreScale == right.modelPreScale &&
        left.animated == right.animated && left.diffuseTextureAssetId == right.diffuseTextureAssetId &&
        left.materialSourceModelAssetId == right.materialSourceModelAssetId &&
        left.animationSetAssetId == right.animationSetAssetId &&
        left.presentationBossArchetypeId == right.presentationBossArchetypeId &&
        left.materialProfile == right.materialProfile && left.mapMaterialBindings == right.mapMaterialBindings;
}

bool_t CWorldSequencePlayer::Admit_PresentationBossModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const TARGET_SET& targets, OBJECT_MODEL& out)
{
    /* Reuse the product boss admission instead of decoding the body again: the
       Level owns the combined body (AnimSet attached), its armour and weapon. */
    const std::string& archetypeId = resource.presentationBossArchetypeId;
    const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
    if (!actor)
    { m_Status = "World Object presentation boss is not in the boss catalog: " + archetypeId; return false; }
    if (actor->bodyModel != resource.modelAssetId || actor->animationSetId != resource.animationSetAssetId ||
        actor->bodyModelPreScale != resource.modelPreScale)
    { m_Status = "World Object body fields must match presentation boss " + archetypeId + ": " + resource.objectId; return false; }
    const wstring_t bodyTag = CValtanPresentationAssetService::Get_BodyModelPrototypeTag(archetypeId);
    if (bodyTag.empty())
    { m_Status = "World Object presentation boss has no product assembly: " + archetypeId; return false; }
    if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(targets.device, targets.context,
        targets.levelIndex, archetypeId)))
    { m_Status = "World Object presentation boss admission failed: " + archetypeId; return false; }
    out.model = dynamic_pointer_cast<CModel>(CGameInstance::Get().Clone_Prototype(targets.levelIndex, bodyTag));
    if (!out.model || !out.model->Get_NumMeshes() || !out.model->Has_Animations())
    { m_Status = "World Object presentation boss body is unavailable: " + Narrow_PrototypeTag(bodyTag); return false; }
    CWorldSequenceObject::DESC parts;
    Fill_PresentationParts(archetypeId, parts);
    for (const auto& part : parts.presentationParts)
    {
        if (!CGameInstance::Get().Clone_Prototype(targets.levelIndex, part.modelPrototypeTag))
        { m_Status = "World Object presentation boss part is unavailable: " + Narrow_PrototypeTag(part.modelPrototypeTag); return false; }
        if (!part.socketBone.empty() && !out.model->Has_Bone(part.socketBone.c_str()))
        { m_Status = "World Object presentation boss socket bone is unavailable: " + part.socketBone; return false; }
    }
    out.presentationBossArchetypeId = archetypeId;
    out.deviceIdentity = targets.device.Get();
    out.contextIdentity = targets.context.Get();
    out.catalogIdentity = targets.pCatalog;
    return true;
}

const CWorldSequencePlayer::OBJECT_MODEL* CWorldSequencePlayer::Find_PreparedObjectModel(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const
{
    const auto matches = [&](const auto& entry) {
        const auto* prepared = m_Document.Find_ObjectResource(entry.first);
        return prepared && Same_ObjectModelInputs(*prepared, resource) && entry.second.model &&
            entry.second.deviceIdentity == targets.device.Get() &&
            entry.second.contextIdentity == targets.context.Get() && entry.second.catalogIdentity == targets.pCatalog;
    };
    // Preserve the exact prototype identity used by an existing object clone pool.
    const auto exact = m_ObjectModels.find(resource.objectId);
    if (exact != m_ObjectModels.end() && matches(*exact)) return &exact->second;
    for (const auto& entry : m_ObjectModels)
        if (matches(entry)) return &entry.second;
    return nullptr;
}

const CWorldSequencePlayer::OBJECT_MODEL* CWorldSequencePlayer::Find_SharedObjectModel(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const
{
    const auto* owner = targets.objectPreparationOwner;
    if (!owner || owner->m_Document.Get_AreaId() != m_Document.Get_AreaId() ||
        owner->m_Document.Get_Revision() != m_Document.Get_Revision()) return nullptr;
    return owner->Find_PreparedObjectModel(resource, targets);
}

void CWorldSequencePlayer::Remember_SharedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const OBJECT_MODEL& model, const TARGET_SET& targets) const
{
    auto* owner = targets.objectPreparationOwner;
    if (!owner || owner == this || owner->m_Document.Get_AreaId() != m_Document.Get_AreaId() ||
        owner->m_Document.Get_Revision() != m_Document.Get_Revision() || !model.model ||
        model.deviceIdentity != targets.device.Get() || model.contextIdentity != targets.context.Get() ||
        model.catalogIdentity != targets.pCatalog) return;
    const auto* prepared = owner->m_Document.Find_ObjectResource(resource.objectId);
    if (!prepared || !Same_ObjectModelInputs(*prepared, resource))
    {
        const auto& resources = owner->m_Document.Get_ObjectResources();
        const auto found = std::find_if(resources.begin(), resources.end(), [&](const auto& row) {
            return Same_ObjectModelInputs(row, resource);
        });
        if (found == resources.end()) return;
        prepared = &*found;
    }
    // The admitted owner document owns this entry and clears it on reload. Each
    // visible object still clones its own CModel/Bones/Animations from the prototype.
    owner->m_ObjectModels.emplace(prepared->objectId, model);
}

bool_t CWorldSequencePlayer::Prepare_ObjectResources(
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    const auto* sequence = m_Document.Find_Template(instance.templateId);
    if (sequence)
        for (const auto& effect : sequence->effectTracks)
        {
            if (effect.resourceKind == "V1_EFFECT")
            {
                const std::vector<std::string> ids{effect.resourceId};
                std::vector<std::string> queued;
                if (!CEffectPresentationService::Queue_ProductTargets_Priority(ids, queued, m_Status)) return false;
                const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(ids);
                if (probe.iFailedCount || probe.iUnavailableCount)
                { m_Status = "World Object V1 effect preparation failed: " + effect.resourceId; return false; }
                if (!probe.bCatalogRevisionCurrent || !probe.bSettled)
                { m_Status = "World Object V1 effect is preparing: " + effect.resourceId; return false; }
                const auto document = CEffectCatalog::Find_Loaded(effect.resourceId);
                const auto binding = std::find_if(instance.bindings.begin(), instance.bindings.end(),
                    [&](const auto& candidate) { return candidate.slotId == effect.slotId &&
                        candidate.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
                const auto* owner = binding == instance.bindings.end() ? nullptr :
                    m_Document.Find_ObjectResource(binding->targetId);
                if (!document || !owner)
                { m_Status = "World Object V1 effect has no prepared document or owner: " + effect.resourceId; return false; }
                for (const auto& cue : document->ModelCues)
                    if (cue.strModelAssetId != owner->modelAssetId)
                    { m_Status = "World Object V1 model cue names a different model: " + cue.strCueId; return false; }
                continue;
            }
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
        if (sequence && binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
        {
            // Reject trimmed native ranges before releasing an existing preview owner.
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId || animation.sourceStartMs == 0u) continue;
                uint64_t targetId = 0u;
                const auto object = Try_ParseTargetId(binding, targetId) && targets.pDeployRuntime ?
                    targets.pDeployRuntime->Find(targetId) : nullptr;
                float duration = 0.f, seconds = 0.f;
                if (object)
                    for (const auto& clip : object->Get_AnimationClips())
                        if (clip.name == animation.clipName) { duration = clip.durationSeconds; break; }
                if (!CWorldSequenceDocument::Try_SampleAnimationTicks(animation, static_cast<float>(animation.startMs),
                    static_cast<float>(sequence->durationMs), 1.f, duration, seconds))
                { m_Status = "World sequence Deploy animation source range is unavailable: " + animation.clipName; return false; }
            }
        }
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        if (!resource || resource->modelAssetId.empty())
        { m_Status = "World Object model binding is unavailable: " + binding.targetId; return false; }
        if (!targets.device || !targets.context)
        { m_Status = "World Object render device is unavailable."; return false; }
        auto model = m_ObjectModels.find(resource->objectId);
        if (model != m_ObjectModels.end() &&
            (model->second.deviceIdentity != targets.device.Get() ||
                model->second.contextIdentity != targets.context.Get() || model->second.catalogIdentity != targets.pCatalog))
        { m_Status = "World Object prepared model belongs to different render targets; reload its owner: " + resource->objectId; return false; }
        if (model == m_ObjectModels.end())
        {
            const auto* prepared = Find_PreparedObjectModel(*resource, targets);
            if (!prepared) prepared = Find_SharedObjectModel(*resource, targets);
            if (prepared) model = m_ObjectModels.emplace(resource->objectId, *prepared).first;
        }
        if (model == m_ObjectModels.end() && !resource->presentationBossArchetypeId.empty())
        {
            OBJECT_MODEL staged;
            if (!Admit_PresentationBossModel(*resource, targets, staged)) return false;
            model = m_ObjectModels.emplace(resource->objectId, std::move(staged)).first;
        }
        if (model == m_ObjectModels.end())
        {
            OBJECT_MODEL staged;
            staged.deviceIdentity = targets.device.Get();
            staged.contextIdentity = targets.context.Get();
            staged.catalogIdentity = targets.pCatalog;
            const auto path = CRuntimeAssetRoot::Resolve(resource->modelAssetId);
            if (path.empty())
            { m_Status = "World Object model path is invalid: " + resource->modelAssetId; return false; }
            MODEL_ASSET_LOAD_DESC load;
            if (!CActorCatalog::Build_DerivedModelLoadDescription(resource->modelAssetId,
                resource->materialSourceModelAssetId, load, m_Status)) return false;
            for (const auto& binding : resource->mapMaterialBindings)
            {
                const auto* asset = targets.pCatalog->Find(binding.sourceAssetId);
                if (!asset) { m_Status = "World Object map material asset is missing: " + binding.sourceAssetId; return false; }
                const auto found = std::find_if(asset->materialOverrides.begin(), asset->materialOverrides.end(),
                    [&](const auto& row) { return row.materialName == binding.sourceMaterialName; });
                if (found == asset->materialOverrides.end() || found->surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
                { m_Status = "World Object map surface binding is missing or unsupported: " + binding.sourceMaterialName; return false; }
                auto material = *found;
                material.materialName = binding.materialName;
                material.surface.sourceBgUnlit = binding.unlit;
                material.surface.hasBakedLighting = false;
                material.surface.hasStaticShadow = false;
                material.bakedAveragePath.clear(); material.bakedDirectionalPath.clear(); material.staticShadowPath.clear();
                if (!binding.diffuseTextureAssetId.empty())
                {
                    material.surfaceDiffusePath = CRuntimeAssetRoot::Resolve(binding.diffuseTextureAssetId);
                    if (material.surfaceDiffusePath.empty()) { m_Status = "World Object surface texture is invalid"; return false; }
                }
                std::erase_if(load.materialOverrides, [&](const auto& prior) { return prior.materialName == material.materialName; });
                load.materialOverrides.push_back(std::move(material));
            }
            if (resource->materialProfile)
            {
                MODEL_MATERIAL_OVERRIDE material;
                if (!CWorldSequenceDocument::Build_MaterialOverride(*resource->materialProfile, load.assetRoot, material))
                { m_Status = "World Object material admission failed: " + resource->objectId; return false; }
                std::erase_if(load.materialOverrides, [&](const MODEL_MATERIAL_OVERRIDE& prior) {
                    return prior.materialName == material.materialName;
                });
                load.materialOverrides.push_back(std::move(material));
            }
            staged.model = CModel::Create(targets.device, targets.context,
                resource->animated ? MODEL::ANIM : MODEL::NONANIM, load,
                XMMatrixScaling(resource->modelPreScale, resource->modelPreScale, resource->modelPreScale));
            if (!staged.model || !staged.model->Get_NumMeshes())
            {
                m_Status = "World Object model admission failed: " + resource->modelAssetId;
                // This Create overload decodes the nonempty load.meshPath on this thread.
                const auto report = CModelDecoderRegistry::Get().Get_LastReport();
                if (report.meshPath == load.meshPath)
                {
                    if (!report.succeeded && !report.error.empty()) m_Status += " / " + report.error;
                    else if (report.succeeded) m_Status += " / binary decoded; model setup failed";
                }
                return false;
            }
            if (!resource->animationSetAssetId.empty())
            {
                /* The Valtan bodies ship their clips in a separate AnimSet WModel.
                   Attach it to the prototype before any clone so every occurrence
                   samples the same clip table the product boss uses. */
                const auto animationSetPath = CRuntimeAssetRoot::Resolve(resource->animationSetAssetId);
                if (animationSetPath.empty())
                { m_Status = "World Object animation set path is invalid: " + resource->animationSetAssetId; return false; }
                const unique_ptr<CModel> animationSet = CModel::Create(targets.device, targets.context,
                    MODEL::ANIM, animationSetPath.string().c_str(),
                    XMMatrixScaling(resource->modelPreScale, resource->modelPreScale, resource->modelPreScale));
                if (!animationSet || !animationSet->Has_Animations() ||
                    FAILED(staged.model->Attach_AnimationSet(*animationSet)))
                { m_Status = "World Object animation set does not match the body: " + resource->animationSetAssetId; return false; }
            }
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
                uint32_t index = UINT32_MAX;
                for (uint32_t i = 0; i < model->second.model->Get_NumAnimations(); ++i)
                    if (animation.clipName == model->second.model->Get_AnimationName(i)) { index = i; break; }
                if (index == UINT32_MAX) { m_Status = "World Object clip is absent: " + animation.clipName; return false; }
                if (animation.sourceStartMs != 0u)
                {
                    float position = 0.f, duration = 0.f, ticks = 0.f;
                    if (!model->second.model->Get_AnimationProgress(index, position, duration) ||
                        !CWorldSequenceDocument::Try_SampleAnimationTicks(animation, static_cast<float>(animation.startMs),
                            static_cast<float>(sequence->durationMs), model->second.model->Get_AnimationTickPerSecond(index), duration, ticks))
                    { m_Status = "World Object animation source start exceeds the native clip range: " + animation.clipName; return false; }
                }
            }
        if (sequence)
            for (const auto& effect : sequence->effectTracks)
            {
                if (effect.slotId != binding.slotId) continue;
                if (!effect.bone.empty() && !model->second.model->Has_Bone(effect.bone.c_str()))
                { m_Status = "World Object Effect bone is unavailable: " + effect.bone; return false; }
                if (effect.resourceKind != "V1_EFFECT") continue;
                const auto document = CEffectCatalog::Find_Loaded(effect.resourceId);
                for (const auto& element : document->Elements)
                {
                    const auto& attachment = element.ActionCueAttachment;
                    if (element.bVisible && attachment.bEnabled && attachment.bFollow &&
                        attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
                        !model->second.model->Has_Bone(attachment.strRuntimeBoneName.c_str()))
                    {
                        // Not a rejection: Sample_ObjectEffectAttachments anchors this slot at the
                        // object pivot, as the owner-anchored product path does for a missing bone.
                        OutputDebugStringA(("[Client][WorldObject] V1 source bone '" + attachment.strRuntimeBoneName +
                            "' is absent on " + resource->modelAssetId + "; slot '" + attachment.strRuntimeAnchorSlotId +
                            "' follows the object pivot (" + effect.resourceId + ")\n").c_str());
                        break;
                    }
                }
            }
        if (sequence)
            for (const auto& collider : sequence->colliderTracks)
                if (collider.slotId == binding.slotId && !collider.attachmentBone.empty() &&
                    !model->second.model->Has_Bone(collider.attachmentBone.c_str()))
                { m_Status = "World Object collider bone is unavailable: " + collider.attachmentBone; return false; }
        Remember_SharedObjectModel(*resource, model->second, targets);
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

bool_t CWorldSequencePlayer::Prewarm_ObjectInstances(const std::string& instanceId,
    const uint32_t copies, const TARGET_SET& targets)
{
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!instance || !sequence || !instance->enabled || instance->anchorKind != "WORLD" ||
        instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
        sequence->objectMotion.EmissionCount() == 0u || sequence->objectMotion.EmissionCount() > 128u || copies == 0u || copies > 128u ||
        (targets.objectPreparationOwner && targets.objectPreparationOwner != this))
    { m_Status = "World Object prewarm requires its owner, one enabled WORLD object binding, 1..128 emissions and copies: " + instanceId; return false; }
    if (!Prepare_InstanceResources(instanceId, targets)) return false;
    const auto& objectId = instance->bindings.front().targetId;
    const auto model = m_ObjectModels.find(objectId);
    if (model == m_ObjectModels.end())
    { m_Status = "World Object prewarm model is unavailable: " + objectId; return false; }
    const auto found = m_PreparedObjectPools.find(objectId);
    auto pool = found == m_PreparedObjectPools.end() ? std::make_shared<PREPARED_OBJECT_POOL>() : found->second;
    if (found != m_PreparedObjectPools.end() && (!pool->acceptsReturns || pool->levelIndex != targets.levelIndex))
    { m_Status = "World Object prewarm belongs to another level; reload its owner: " + objectId; return false; }
    if (pool->capacity >= copies)
    { m_Status = "World Object clones already prepared: " + objectId + " / " + std::to_string(pool->capacity); return true; }
    size_t total = copies - pool->capacity;
    for (const auto& [id, prepared] : m_PreparedObjectPools) total += prepared->capacity;
    if (total > 1024u) { m_Status = "World Object prepared clone budget reached (1024)."; return false; }
    pool->idle.reserve(copies);
    std::vector<shared_ptr<CWorldSequenceObject>> staged;
    const auto rollback = [&]() {
        for (auto& object : staged)
        {
            object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, object);
        }
    };
    for (uint32_t index = pool->capacity; index < copies; ++index)
    {
        CWorldSequenceObject::DESC desc;
        desc.levelIndex = targets.levelIndex;
        desc.modelPrototype = model->second.model;
        desc.diffuseTexture = model->second.diffuse;
        Fill_PresentationParts(model->second.presentationBossArchetypeId, desc);
        shared_ptr<CGameObject> created;
        const HRESULT result = CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
            CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex, CWorldSequenceObject::LAYER_TAG, &desc, &created);
        auto object = dynamic_pointer_cast<CWorldSequenceObject>(created);
        if (FAILED(result) || !object)
        {
            if (created) CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, created);
            rollback();
            m_Status = "World Object prewarm clone/shader creation failed: " + objectId +
                " / copy " + std::to_string(index + 1u) + " / HRESULT " + std::to_string(static_cast<int32_t>(result));
            return false;
        }
        object->Hide();
        staged.push_back(std::move(object));
    }
    pool->idle.insert(pool->idle.end(), staged.begin(), staged.end());
    pool->levelIndex = targets.levelIndex;
    pool->capacity = copies;
    m_PreparedObjectPools.insert_or_assign(objectId, std::move(pool));
    m_Status = "World Object hidden clones prepared: " + objectId + " / " + std::to_string(copies);
    return true;
}

void CWorldSequencePlayer::Clear_PreparedObjects()
{
    for (auto& [id, pool] : m_PreparedObjectPools)
    {
        // Borrowed clones keep this token alive. Once the owner resets, they
        // remove themselves on release instead of returning to an obsolete pool.
        pool->acceptsReturns = false;
        for (auto& object : pool->idle)
        {
            object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(pool->levelIndex, CWorldSequenceObject::LAYER_TAG, object);
        }
        pool->idle.clear();
    }
    m_PreparedObjectPools.clear();
}

void CWorldSequencePlayer::Release_Objects(ACTIVE_INSTANCE& active)
{
#ifdef _DEBUG
    active.objectColliderSamples.clear();
#endif
    for (const auto& effect : active.effects)
    {
        if (effect.handle) CEffectV2Runtime::Stop_Group(effect.handle);
        if (effect.v1Handle) CEffectPresentationService::Stop_WorldRoot({effect.v1Handle});
    }
    active.effects.clear();
    active.emissionAnchors.clear();
    for (auto& entry : active.objects)
    {
        entry.weaponReplacement.reset();
        entry.hatReplacement.reset();
        if (entry.object)
        {
            entry.object->Hide();
            auto& pool = entry.preparationPool;
            if (pool && pool->acceptsReturns && pool->levelIndex == entry.levelIndex && pool->idle.size() < pool->capacity)
            {
                if (entry.object->Reset_ForReuse())
                {
                    pool->idle.push_back(std::move(entry.object));
                    continue;
                }
                --pool->capacity;
                m_Status = "World Object prepared clone could not return to its pool: " +
                    (entry.object->Get_RenderStatus().empty() ? entry.slotId : entry.object->Get_RenderStatus());
            }
            CGameInstance::Get().Remove_GameObject_from_Layer(entry.levelIndex,
                CWorldSequenceObject::LAYER_TAG, entry.object);
        }
    }
    active.objects.clear();
}

bool_t CWorldSequencePlayer::Try_GetPresentationBossAnchor(const std::string& archetype,
    const std::string& bone, PLAYER_ANCHOR& out, std::string& status) const
{
    status.clear();
    const CWorldSequenceObject* selected = nullptr;
    bool declared = false;
    for (const auto& active : m_Active)
    {
        const auto* instance = m_Document.Find_Instance(active.instanceId);
        if (!instance) continue;
        for (const auto& binding : instance->bindings)
        {
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
            const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
            if (!resource) continue;
            const auto* actor = CActorCatalog::Find_Boss(archetype);
            const bool derivedActor = actor && resource->animated &&
                resource->materialSourceModelAssetId == actor->bodyModel &&
                std::abs(resource->modelPreScale - actor->bodyModelPreScale) < .0000001f;
            if (resource->presentationBossArchetypeId != archetype && !derivedActor) continue;
            declared = true;
            for (const auto& entry : active.objects)
            {
                if (entry.slotId != binding.slotId || !entry.object || !entry.object->Is_Visible()) continue;
                if (selected && selected != entry.object.get())
                { status = "Cinematic World boss anchor is ambiguous: " + archetype; return false; }
                selected = entry.object.get();
            }
        }
    }
    if (!selected)
    {
        if (declared) status = "Cinematic World boss anchor is waiting for its sampled actor: " + archetype;
        return false;
    }
    if (!Resolve_BossBoneAnchor(selected->Get_Model(), selected->Get_SampledWorld(), bone, out, status)) return false;
    out.liveBossAnchor = true;
    return true;
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

#ifdef _DEBUG
void CWorldSequencePlayer::Collect_ObjectColliderSamples(std::vector<OBJECT_COLLIDER_SAMPLE>& out) const
{
    out.clear();
    for (const auto& active : m_Active)
        out.insert(out.end(), active.objectColliderSamples.begin(), active.objectColliderSamples.end());
}
#endif

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
    if (!targets.objectEmissionAnchor || baseline.liveBossAnchor) return true;
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
    const PLAYER_ANCHOR& anchor, const uint32_t emitter, const f32_t ageMs, float4x4_t& out, std::string& status,
    const bool_t inheritObjectRotation)
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
    // Sample after the existing velocity draws so zero extents preserve old paths.
    // Seed + emitter remain stable across every age sample and attached Effect.
    const float spawnX = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.x;
    const float spawnY = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.y;
    const float spawnZ = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.z;
    const vector_t spawnOffset = XMVectorSet(spawnX, spawnY, spawnZ, 0.f);
    const matrix_t revolution = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(motion.revolutionDegreesPerSecond.x * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.y * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.z * seconds));
    const vector_t orbit = XMLoadFloat3(&motion.revolutionOffset);
    const bool hasAuthoredEmissions = !motion.emissions.empty();
    const bool hasInstanceOffset = !((active.placement && instance.anchorKind == "WORLD") || anchor.emissionOverride);
    // Legacy emitters keep their existing addition order. Authored rows rotate
    // their local motion first, then add the instance's untranslated ring centre.
    const vector_t instanceOffset = hasInstanceOffset ? XMLoadFloat3(&instance.position) : XMVectorZero();
    const vector_t position = (hasAuthoredEmissions ? XMVectorZero() : instanceOffset) + XMLoadFloat3(&key.positionOffset) + spawnOffset +
        velocity * seconds + XMLoadFloat3(&motion.acceleration) * (.5f * seconds * seconds) +
        XMVector3TransformNormal(orbit, revolution) - orbit;
    const vector_t scale = XMLoadFloat3(&resource.scale) * XMLoadFloat3(&key.scaleMultiplier);
    matrix_t basis = XMLoadFloat4x4(&anchor.world);
    for (int axis = 0; axis < 3; ++axis)
    {
        const float length = XMVectorGetX(XMVector3LengthSq(basis.r[axis]));
        if (!std::isfinite(length) || length < 1e-6f)
        { status = "World Object anchor transform is invalid: " + resource.objectId; return false; }
        basis.r[axis] = XMVectorSetW(XMVector3Normalize(basis.r[axis]), 0.f);
    }
    // Floor colliders use emission + placement facing, without mesh upright correction or self-spin.
    // Attached effects can share that basis without changing the visible model sample.
    const matrix_t rotation = inheritObjectRotation ? XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(motion.angularVelocityDegrees.x * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.y * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.z * seconds)) : XMMatrixIdentity();
    matrix_t world = XMMatrixScalingFromVector(scale) * rotation * XMMatrixTranslationFromVector(position);
    /* An authored row turns the whole local motion, orbit and seeded spawn
       offset included, before the existing placement / live-anchor composition. */
    if (hasAuthoredEmissions)
    {
        const auto& emission = motion.emissions[(std::min)(static_cast<size_t>(emitter), motion.emissions.size() - 1u)];
        world *= XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&emission.positionOffset));
        if (hasInstanceOffset)
            world *= XMMatrixTranslationFromVector(instanceOffset);
    }
    const bool localPlacement = active.placement && (instance.anchorKind == "BOSS" || instance.anchorKind == "PLAYER");
    if (!anchor.emissionOverride && !localPlacement) world *= basis;
    if (active.placement)
    {
        const auto& placement = *active.placement;
        world *= XMMatrixScalingFromVector(XMLoadFloat3(&placement.scale)) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(placement.rotationDegrees.x),
                XMConvertToRadians(placement.rotationDegrees.y), XMConvertToRadians(placement.rotationDegrees.z)) *
            (anchor.emissionOverride ? XMMatrixIdentity() : XMMatrixTranslationFromVector(XMLoadFloat3(&placement.position)));
    }
    else if (!anchor.emissionOverride) world.r[3] += XMVectorSet(active.positionOffset.x, active.positionOffset.y, active.positionOffset.z, 0.f);
    if (anchor.emissionOverride || localPlacement) world *= basis;
    XMStoreFloat4x4(&out, world);
    return true;
}

bool_t CWorldSequencePlayer::Apply_Objects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const TARGET_SET& targets, const f32_t localMs, const bool_t visible, const bool_t holdFinalPose,
    const f32_t emissionStartMs, const f32_t emissionRate, const std::string& emissionMotionId)
{
    active.objectSampleStatus.clear();
#ifdef _DEBUG
    active.objectColliderSamples.clear();
    std::vector<OBJECT_COLLIDER_SAMPLE> colliderSamples;
#endif
    for (auto& entry : active.objects) entry.object->Hide();
    if (!visible || std::none_of(instance.bindings.begin(), instance.bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; })) return true;
    std::vector<PLAYER_ANCHOR> anchors;
    if (instance.anchorKind == "BOSS")
    {
        const auto* resource = m_Document.Find_ObjectResource(instance.bindings.front().targetId);
        PLAYER_ANCHOR anchor;
        anchor.liveBossAnchor = true;
        if (!resource || !targets.bossAnchor || !targets.bossAnchor(resource->anchorBossArchetypeId,
            resource->anchorBone, anchor, active.objectSampleStatus))
        {
            if (active.objectSampleStatus.empty()) active.objectSampleStatus = "World Object Boss anchor is unavailable.";
            m_Status = active.objectSampleStatus;
            return true;
        }
        anchors.push_back(anchor);
    }
    else if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
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
                    shared_ptr<CWorldSequenceObject> object;
                    std::shared_ptr<PREPARED_OBJECT_POOL> pool;
                    const auto* prepared = Find_SharedObjectModel(*resource, targets);
                    if (prepared && prepared->model == model->second.model)
                    {
                        const auto available = targets.objectPreparationOwner->m_PreparedObjectPools.find(resource->objectId);
                        if (available != targets.objectPreparationOwner->m_PreparedObjectPools.end() &&
                            available->second->acceptsReturns && available->second->levelIndex == targets.levelIndex &&
                            !available->second->idle.empty())
                        {
                            pool = available->second;
                            object = std::move(pool->idle.back());
                            pool->idle.pop_back();
                        }
                    }
                    if (!object)
                    {
                        CWorldSequenceObject::DESC desc;
                        desc.levelIndex = targets.levelIndex;
                        desc.modelPrototype = model->second.model;
                        desc.diffuseTexture = model->second.diffuse;
                        Fill_PresentationParts(model->second.presentationBossArchetypeId, desc);
                        shared_ptr<CGameObject> staged;
                        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
                            CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex,
                            CWorldSequenceObject::LAYER_TAG, &desc, &staged)))
                        { m_Status = "World Object clone/shader creation failed: " + resource->objectId; return false; }
                        object = dynamic_pointer_cast<CWorldSequenceObject>(staged);
                        if (!object)
                        {
                            CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, staged);
                            m_Status = "World Object clone type is invalid: " + resource->objectId; return false;
                        }
                    }
                    active.objects.push_back({binding.slotId, anchor.entityId, emitter, targets.levelIndex,
                        std::move(object), std::move(pool)});
                    found = active.objects.end() - 1;
                }
                const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
                float4x4_t stored;
                if (!Sample_ObjectWorld(active, instance, sequence, *resource, binding.slotId, emissionAnchor, emitter,
                    (std::min)(ageMs, static_cast<f32_t>(sequence.durationMs)), stored, m_Status)) return false;
                f32_t windowEnd = 0.f;
                const auto* animation = Find_AnimationTrackAt(sequence, binding.slotId, ageMs, windowEnd);
                if (!found->object)
                { m_Status = "World Object clone type is invalid: " + resource->objectId; return false; }
                if (!found->object->Get_RenderStatus().empty())
                { m_Status = found->object->Get_RenderStatus() + " / " + resource->objectId; return false; }
                if (!found->object->Sample(stored, key.visible, animation, ageMs, windowEnd))
                { m_Status = "World Object transform/animation sample failed: " + resource->objectId; return false; }
#ifdef _DEBUG
                if (found->object->Is_Visible())
                    for (const auto& collider : sequence.colliderTracks)
                    {
                        if (collider.slotId != binding.slotId || ageMs < collider.startMs ||
                            ageMs >= static_cast<double>(collider.startMs) + collider.durationMs) continue;
                        OBJECT_COLLIDER_SAMPLE sample;
                        if (!Sample_ObjectCollider(collider, *resource, key, motion, emitter,
                            active.placement, *found->object, sample))
                        { m_Status = "World Object collider attachment sample failed: " + collider.colliderTrackId; return false; }
                        sample.instanceId = active.instanceId;
                        sample.colliderTrackId = collider.colliderTrackId;
                        sample.emissionIndex = emitter;
                        colliderSamples.push_back(std::move(sample));
                    }
#endif
                if (anchor.liveBossAnchor &&
                    (resource->objectId == "world.object.kouku.saydon_showtime_gun_left" ||
                     resource->objectId == "world.object.kouku.saydon_showtime_gun_right"))
                    CNpcPresentationAssetService::Track_SaydonWeaponReplacement(found->weaponReplacement,
                        anchor.bodyModel, found->object);
                else found->weaponReplacement.reset();
                if (anchor.liveBossAnchor && resource->objectId == "world.object.kouku.saydon_hat_right")
                    CNpcPresentationAssetService::Track_SaydonHatReplacement(found->hatReplacement,
                        anchor.bodyModel, found->object);
                else found->hatReplacement.reset();
            }
    }
#ifdef _DEBUG
    active.objectColliderSamples = std::move(colliderSamples);
#endif
    return true;
}

bool_t CWorldSequencePlayer::Apply_ObjectEffects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    std::vector<PLAYER_ANCHOR> anchors;
    if (instance.anchorKind == "BOSS")
    {
        const auto* resource = m_Document.Find_ObjectResource(instance.bindings.front().targetId);
        PLAYER_ANCHOR anchor;
        anchor.liveBossAnchor = true;
        if (resource && targets.bossAnchor && targets.bossAnchor(resource->anchorBossArchetypeId,
            resource->anchorBone, anchor, active.objectSampleStatus)) anchors.push_back(anchor);
    }
    else if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
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
            const f32_t period = static_cast<f32_t>(motion->CycleSpanMs(*sequence));
            for (const auto& effect : sequence->effectTracks)
            {
                // Each effect owns its declared slot, including a different
                // skeleton/scale in a multi-actor cinematic or a NEXT motion.
                const auto binding = std::find_if(motion->bindings.begin(), motion->bindings.end(),
                    [&](const auto& candidate) { return candidate.slotId == effect.slotId &&
                        candidate.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
                if (binding == motion->bindings.end())
                { m_Status = "World Object effect slot has no object binding: " + effect.slotId; return false; }
                const auto* resource = m_Document.Find_ObjectResource(binding->targetId);
                if (!resource)
                { m_Status = "World Object effect resource is unavailable: " + binding->targetId; return false; }
                const auto snapshot = m_EffectSnapshots.find(effect.resourceKind + ":" + effect.resourceId);
                if (effect.resourceKind != "V1_EFFECT" && snapshot == m_EffectSnapshots.end())
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
                            auto key = firstId + ":" + motion->instanceId + ":" + std::to_string(firstStartMs) + ":" +
                                effect.effectTrackId + ":" + std::to_string(epoch) + ":" + std::to_string(emitter) + ":" + std::to_string(anchor.entityId);
                            PLAYER_ANCHOR emissionAnchor;
                            const f32_t emissionStartMs = start + static_cast<f32_t>(epoch) * period / rate;
                            const auto emissionKey = motion->instanceId + ":" + std::to_string(emissionStartMs) + ":" + std::to_string(emitter);
                            if (!Get_EmissionAnchor(active, targets, emissionKey, birthMs, anchor, emissionAnchor)) return false;
                            const auto prepared = m_ObjectModels.find(resource->objectId);
                            if (prepared == m_ObjectModels.end())
                            { m_Status = "World Object Effect model was not prepared: " + resource->objectId; return false; }
                            const bool v1 = effect.resourceKind == "V1_EFFECT";
                            const auto document = !v1 ? nullptr : CEffectCatalog::Find_Loaded(effect.resourceId);
                            if (v1 && !document)
                            { m_Status = "World Object V1 Effect document was not prepared: " + effect.resourceId; return false; }
                            float effectTimeScale = 1.f;
                            if (effect.fitEffectToDuration)
                            {
                                float sourceDuration = 0.f;
                                if (!CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(effect.resourceId, sourceDuration) ||
                                    !CWorldSequenceDocument::Try_EffectTimeScale(effect, sourceDuration, effectTimeScale))
                                { m_Status = "World Object Effect fit needs a finite prepared V1 duration: " + effect.resourceId; return false; }
                            }
                            double sourceCycleMs = 0.0;
                            uint64_t firstSourceCycle = 0u, lastSourceCycle = 0u;
                            bool nativeInfiniteLoop = false;
                            if (effect.loopEffectToDuration)
                            {
                                nativeInfiniteLoop = std::any_of(document->Elements.begin(), document->Elements.end(),
                                    [](const auto& element) { return element.SourceRecipe.bEnabled &&
                                        element.SourceRecipe.iEmitterLoopCount == 0u; });
                                if (!nativeInfiniteLoop)
                                {
                                    float sourceDuration = 0.f;
                                    if (!CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(effect.resourceId, sourceDuration) ||
                                        !std::isfinite(sourceDuration) || sourceDuration <= 0.f)
                                    { m_Status = "World Object Effect loop needs a finite prepared V1 duration: " + effect.resourceId; return false; }
                                    // A prepared duration includes particle/after-image tails. Repeat
                                    // emission at its authored end, retaining each still-living cycle.
                                    double emissionSeconds = 0.0;
                                    for (const auto& element : document->Elements)
                                    {
                                        if (!element.bVisible) continue;
                                        const auto& recipe = element.SourceRecipe;
                                        const auto& timing = element.Detail.Timing;
                                        const double duration = recipe.bEnabled && recipe.fEmitterDurationSeconds > 0.f ?
                                            static_cast<double>(recipe.fEmitterDurationSeconds) * recipe.iEmitterLoopCount :
                                            timing.fLifeTimeSeconds;
                                        emissionSeconds = (std::max)(emissionSeconds,
                                            timing.fStartDelaySeconds + (recipe.bEnabled ? recipe.fEmitterDelaySeconds : 0.f) + duration);
                                    }
                                    for (const auto& cue : document->ModelCues)
                                        if (cue.bVisible) emissionSeconds = (std::max)(emissionSeconds,
                                            static_cast<double>(cue.fStartDelaySeconds) + cue.fDurationSeconds);
                                    if (!std::isfinite(emissionSeconds) || emissionSeconds <= 0.0)
                                    { m_Status = "World Object Effect loop has no finite emission window: " + effect.resourceId; return false; }
                                    sourceCycleMs = (std::min)(emissionSeconds, static_cast<double>(sourceDuration)) * 1000.0;
                                    const double lastSourceEpoch = std::floor(ageMs / sourceCycleMs);
                                    const double firstSourceEpoch = (std::max)(0.0,
                                        std::floor((ageMs - sourceDuration * 1000.0) / sourceCycleMs) + 1.0);
                                    if (!std::isfinite(lastSourceEpoch) ||
                                        lastSourceEpoch >= static_cast<double>((std::numeric_limits<uint64_t>::max)()) ||
                                        lastSourceEpoch - firstSourceEpoch >= 1024.0)
                                    { m_Status = "World Object Effect source tails exceed the bounded occurrence range."; return false; }
                                    firstSourceCycle = static_cast<uint64_t>(firstSourceEpoch);
                                    lastSourceCycle = static_cast<uint64_t>(lastSourceEpoch);
                                }
                            }
                            const auto occurrenceKey = key;
                            for (uint64_t sourceCycle = firstSourceCycle; sourceCycle <= lastSourceCycle; ++sourceCycle)
                            {
                                const float sourceCycleStartMs = static_cast<float>(sourceCycle * sourceCycleMs);
                                key = occurrenceKey;
                                if (sourceCycleMs > 0.0) key += ":source-cycle:" + std::to_string(sourceCycle);
                                wanted.insert(key);
                                auto found = std::find_if(active.effects.begin(), active.effects.end(),
                                    [&](const auto& value) { return value.key == key; });
                                const float sourceSeconds = (std::max)(0.f, ageMs - sourceCycleStartMs) * .001f * effectTimeScale;
                                ACTIVE_INSTANCE placementState;
                                placementState.positionOffset = active.positionOffset;
                                placementState.placement = active.placement;
                                // This provider outlives this stack frame in PresentationService. All
                                // inputs are owned values or immutable prepared model/document handles.
                                const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER provider =
                                    [placementState, owner = *motion, sequence = *sequence, resource = *resource,
                                     effect, emissionAnchor, emitter, trigger, effectTimeScale, sourceCycleStartMs,
                                     model = prepared->second.model, document, v1]
                                    (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error) -> bool_t
                                {
                                    output.SourceAnchorWorlds.clear();
                                    if (!std::isfinite(seconds) || seconds < 0.f) return false;
                                    const float sampleMs = (std::min)(trigger + (effect.followObject ?
                                        sourceCycleStartMs + seconds * 1000.f / effectTimeScale : 0.f),
                                        static_cast<float>(sequence.durationMs));
                                    float4x4_t objectWorld;
                                    if (!Sample_ObjectWorld(placementState, owner, sequence, resource, effect.slotId,
                                        emissionAnchor, emitter, sampleMs, objectWorld, error, effect.inheritObjectRotation)) return false;
                                    matrix_t pivot = XMLoadFloat4x4(&objectWorld);
                                    // Preserve existing V2 metre sizing; V1 shares the Object's
                                    // authored owner scale so both doll variants attach proportionally.
                                    if (!v1) for (int axis = 0; axis < 3; ++axis)
                                        pivot.r[axis] = XMVectorSetW(XMVector3Normalize(pivot.r[axis]), 0.f);
                                    if (!effect.bone.empty())
                                    {
                                        float4x4_t bone;
                                        if (!Sample_ObjectEffectBone(model, sequence, effect.slotId, effect.bone, sampleMs, bone, error)) return false;
                                        pivot = XMLoadFloat4x4(&bone) * pivot;
                                    }
                                    const matrix_t local = XMMatrixScalingFromVector(XMLoadFloat3(&effect.scale)) *
                                        XMMatrixRotationRollPitchYaw(XMConvertToRadians(effect.rotationDegrees.x),
                                            XMConvertToRadians(effect.rotationDegrees.y), XMConvertToRadians(effect.rotationDegrees.z)) *
                                        XMMatrixTranslationFromVector(XMLoadFloat3(&effect.positionOffset));
                                    XMStoreFloat4x4(&output.RootWorld, local * pivot);
                                    return !document || Sample_ObjectEffectAttachments(*document, model, sequence, effect.slotId,
                                        sampleMs, output.RootWorld, output.SourceAnchorWorlds, error);
                                };
                                EFFECT_FIXED_STEP_TRANSFORM_SAMPLE frame;
                                if (!provider(sourceSeconds, frame, m_Status)) return false;
                                if (found == active.effects.end())
                                {
                                    size_t total = active.effects.size();
                                    for (const auto& value : m_Active) if (&value != &active) total += value.effects.size();
                                    if (total >= 1024u)
                                    { m_Status = "World Object Effect occurrence budget reached (1024)."; return false; }
                                    if (v1)
                                    {
                                        EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
                                        spawn.iLevelIndex = targets.levelIndex;
                                        // Handle identity is process-local; no pointer is serialized.
                                        spawn.strPlacementId = "world-object:" + std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ":" + key;
                                        spawn.strEffectAssetId = effect.resourceId;
                                        spawn.RootWorld = frame.RootWorld;
                                        spawn.fInitialSampleTimeSeconds = sourceSeconds;
                                        // Finite sources keep their prepared document and repeat above. Native
                                        // EmitterLoops=0 sources use the shared bounded emission policy.
                                        spawn.fSourceLoopEndSeconds = nativeInfiniteLoop ? effect.durationMs * .001f : 0.f;
                                        spawn.bExternallySampled = true;
                                        spawn.bExternalModelCueAnchors = !document->ModelCues.empty();
                                        EFFECT_WORLD_ROOT_HANDLE handle;
                                        if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_Status)) return false;
                                        active.effects.push_back({key, 0u, handle.iValue, document});
                                    }
                                    else
                                    {
                                        EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                                        playback.PivotWorld = frame.RootWorld;
                                        playback.bExternalClock = true;
                                        playback.bProductOwned = true;
                                        // Sample_Group receives authored seconds; Motion speed is already applied.
                                        playback.fDurationSeconds = -1.f;
                                        const uint32_t handle = effect.resourceKind == "GROUP" ?
                                            CEffectV2Runtime::Play_Group(*snapshot->second->Find_Group(effect.resourceId), snapshot->second,
                                                playback, targets.device, targets.context) :
                                            CEffectV2Runtime::Play_Leaf(effect.resourceId, snapshot->second, playback, targets.device, targets.context);
                                        if (!handle)
                                        { m_Status = "World Object effect play failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                                        active.effects.push_back({key, handle});
                                    }
                                    found = active.effects.end() - 1;
                                }
                                if (v1)
                                {
                                    const bool placementEdited = found->sampledPlacement != active.placement ||
                                        found->sampledPositionOffset.x != active.positionOffset.x ||
                                        found->sampledPositionOffset.y != active.positionOffset.y ||
                                        found->sampledPositionOffset.z != active.positionOffset.z;
                                    if (!CEffectPresentationService::Update_WorldRoot({found->v1Handle}, frame.RootWorld) ||
                                        !CEffectPresentationService::Seek_WorldRoot({found->v1Handle}, sourceSeconds, provider, placementEdited))
                                    { m_Status = "World Object V1 effect sample failed: " + effect.resourceId + " / " + CEffectPresentationService::Get_Status(); return false; }
                                    found->sampledPlacement = active.placement;
                                    found->sampledPositionOffset = active.positionOffset;
                                }
                                else
                                {
                                    CEffectV2Runtime::Set_GroupPivot(found->handle, frame.RootWorld);
                                    if (!CEffectV2Runtime::Sample_Group(found->handle, ageMs * .001f, true, targets.device, targets.context))
                                    { m_Status = "World Object effect sample failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                                }
                            }
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
        if (active.effects[index].handle) CEffectV2Runtime::Stop_Group(active.effects[index].handle);
        if (active.effects[index].v1Handle) CEffectPresentationService::Stop_WorldRoot({active.effects[index].v1Handle});
        active.effects.erase(active.effects.begin() + static_cast<ptrdiff_t>(index));
    }
    return true;
}

bool_t Client::CWorldSequencePlayer::Try_GetSequencePivot(const std::string& instanceId, float4x4_t& out,
 const uint32_t emissionIndex) const
{
 if (Try_GetObjectPivot(instanceId, out, emissionIndex)) return true;
 // Placed map/deploy aliases have no emission rows; only row 0 can name them.
 if (0u != emissionIndex) return false;
 const auto* instance = Get_Document().Find_Instance(instanceId);
 if (!instance) return false;
 const WORLD_SEQUENCE_BINDING* binding = nullptr;
 // Preserve the existing map-alias choice; a Deploy-only sequence resolves its animated prop.
 for (const auto kind : {WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT, WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT})
 {
  for (const auto& candidate : instance->bindings)
  {
   if (candidate.targetKind != kind) continue;
   if (candidate.slotId == "object") { binding = &candidate; break; }
   if (binding) return false;
   binding = &candidate;
  }
  if (binding) break;
 }
 if (!binding) return false;
 uint64_t placementId = 0;
 if (!CWorldSequencePlayer::Try_ParseTargetId(*binding, placementId)) return false;
 if (binding->targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
 {
  const auto active = std::find_if(m_Active.begin(), m_Active.end(),
   [&](const auto& value) { return value.instanceId == instanceId; });
  if (active == m_Active.end()) return false;
  const auto sampled = active->sampledDeployPivots.find(placementId);
  if (sampled == active->sampledDeployPivots.end()) return false;
  out = sampled->second;
  return true;
 }
 MAP_PLACEMENT_RECORD record;
 if (!Try_GetSampledPlacementRecord(instanceId, placementId, record)) return false;
 XMStoreFloat4x4(&out, XMMatrixScaling(record.signedScale.x, record.signedScale.y, record.signedScale.z) *
  XMMatrixRotationQuaternion(XMLoadFloat4(&record.rotationQuaternion)) *
  XMMatrixTranslation(record.position.x, record.position.y, record.position.z));
 return true;
}
