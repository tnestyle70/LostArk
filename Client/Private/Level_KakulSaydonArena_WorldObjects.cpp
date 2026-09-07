#include "Level_KakulSaydonArena.h"
#include "Character.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "Transform.h"
#include <cmath>
#include <algorithm>

using namespace Client;

CWorldSequencePlayer::TARGET_SET CLevel_KakulSaydonArena::Make_WorldSequenceTargets()
{
    CWorldSequencePlayer::TARGET_SET targets;
    targets.levelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
    targets.pCatalog = &m_MapRuntime.Get_Catalog();
    targets.pPlacements = &m_MapRuntime.Get_MutablePlacements();
    targets.pDeployRuntime = &m_DeployRuntime;
    targets.device = m_pDevice;
    targets.context = m_pContext;
    targets.playerAnchors = [this]()
    {
        std::vector<CWorldSequencePlayer::PLAYER_ANCHOR> anchors;
        std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
        std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
        m_Replication.Collect_KoukuPresentationViews(bosses, players);
        for (const auto& player : players)
        {
            const auto character = player.pCharacter.lock();
            if (!player.Snapshot.iCurrentHp || !character || !character->Get_Transform()) continue;
            anchors.push_back({player.Snapshot.iNetEntityId, *character->Get_Transform()->Get_WorldMatrixPtr()});
        }
        return anchors;
    };
    return targets;
}

void CLevel_KakulSaydonArena::Get_WorldObjectValidationTargets(
    WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy) const
{
    CWorldSequencePlayer::Collect_ValidationTargets(
        const_cast<CLevel_KakulSaydonArena*>(this)->Make_WorldSequenceTargets(), placements, deploy);
}

bool_t CLevel_KakulSaydonArena::Try_GetWorldSequencePlacementBaseline(
    const WORLD_SEQUENCE_INSTANCE& instance, float3_t& outPosition) const
{
    const WORLD_SEQUENCE_BINDING* primary = nullptr;
    const WORLD_SEQUENCE_BINDING* single = nullptr;
    size_t placementCount = 0;
    for (const auto& binding : instance.bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT) continue;
        ++placementCount;
        single = &binding;
        if (binding.slotId == "object") primary = &binding;
    }
    const auto* binding = primary ? primary : placementCount == 1 ? single : nullptr;
    uint64_t id = 0;
    if (!binding || !CWorldSequencePlayer::Try_ParseTargetId(*binding, id)) return false;
    for (const auto& entry : m_MapRuntime.Get_Placements())
    {
        if (entry.record.placementId != id) continue;
        const auto& position = entry.record.position;
        if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return false;
        outPosition = position;
        return true;
    }
    return false;
}

bool_t CLevel_KakulSaydonArena::Reload_WorldObjectRuntime(std::string& status)
{
    if (m_SequencePlayer.Has_ActiveInstances())
    { status = "Wait for the active Server world sequence before reloading."; return false; }
#ifdef _DEBUG
    Debug_StopWorldObjectPreview();
    Debug_StopCompositionWorldPreview();
#endif
    const bool_t result = m_SequencePlayer.Load_Area("LV_LUT_MIDNIGHTC_ED", Make_WorldSequenceTargets());
    status = m_SequencePlayer.Get_Status();
    return result;
}

#ifdef _DEBUG
bool_t CLevel_KakulSaydonArena::Debug_BeginWorldObjectPreview(
    const CWorldSequenceDocument& document, const std::string& instanceId, std::string& status,
    const bool_t previewAtCharacter)
{
    if (m_SequencePlayer.Has_ActiveInstances() || m_pCompositionWorldPreview)
    { status = "Stop the active pattern/world preview before previewing this object."; return false; }
    const auto targets = Make_WorldSequenceTargets();
    auto staged = std::make_unique<CWorldSequencePlayer>();
    if (!staged->Set_Document(document, targets, status) || !staged->Prepare_InstanceResources(instanceId, targets))
    { status = staged->Get_Status(); return false; }
    float3_t previewOffset{};
    const auto* instance = document.Find_Instance(instanceId);
    const bool hasModels = instance && std::any_of(instance->bindings.begin(), instance->bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
    if (previewAtCharacter && hasModels && instance->anchorKind == "WORLD")
    {
        float3_t previewPosition{};
        if (!Try_Get_AuthoringPreviewPlacement(previewPosition, status)) return false;
        previewOffset = {previewPosition.x - instance->position.x,
            previewPosition.y - instance->position.y, previewPosition.z - instance->position.z};
    }
    // The preview offset never edits the saved map anchor or placed curtain/roulette.
    // All resources and the preview placement are admitted before releasing the old presentation.
    Debug_StopWorldObjectPreview();
    if (!staged->Play(instanceId, targets, 1.f, previewOffset)) { status = staged->Get_Status(); return false; }
    m_pWorldObjectPreview = std::move(staged);
    m_WorldObjectPreviewInstance = instanceId;
    return Debug_SampleWorldObjectPreview(0.f, status);
}

bool_t CLevel_KakulSaydonArena::Debug_SampleWorldObjectPreview(const f32_t clockMs, std::string& status)
{
    if (!m_pWorldObjectPreview || !std::isfinite(clockMs) || clockMs < 0.f)
    { status = "World Object preview is not active."; return false; }
    if (!m_pWorldObjectPreview->Seek_InstanceToMs(m_WorldObjectPreviewInstance, clockMs, Make_WorldSequenceTargets()))
    {
        status = m_pWorldObjectPreview->Get_Status();
        Debug_StopWorldObjectPreview();
        return false;
    }
    status = m_pWorldObjectPreview->Get_ObjectSampleStatus(m_WorldObjectPreviewInstance);
    return true;
}

void CLevel_KakulSaydonArena::Debug_StopWorldObjectPreview()
{
    if (m_pWorldObjectPreview) m_pWorldObjectPreview->Stop_All(Make_WorldSequenceTargets(), true);
    m_pWorldObjectPreview.reset();
    m_WorldObjectPreviewInstance.clear();
}
#endif
