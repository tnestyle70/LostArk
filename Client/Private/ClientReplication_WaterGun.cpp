#include "ClientReplication.h"

#include "ActionPresentationTimeline.h"
#include "Character.h"
#include "Npc.h"
#include "Transform.h"
#include "Effect_PresentationService.h"
#include "Gameplay/MaharakaWaterpangContract.h"

#include <cmath>

namespace
{
    const char* Slot(const std::string_view archetype)
    {
        if (archetype == "maharaka.watergun.burst") return "q";
        if (archetype == "maharaka.watergun.bomb") return "w";
        if (archetype == "maharaka.watergun.single") return "r";
        return nullptr;
    }

    std::string Asset(const std::string_view archetype, const char* phase)
    {
        const char* slot = Slot(archetype);
        return slot ? std::string("effect.maharaka.watergun.") + slot + "." + phase : std::string{};
    }
}

bool Client::CClientReplication::Spawn_WaterGunProjectile(
    const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned,
    COMBAT_OBJECT_PRESENTATION_HANDLE& handle, std::string& status)
{
    if (m_Desc.iLayerLevelIndex != ETOUI(LEVEL::MAHARAKA) || !Slot(spawned.strCombatObjectArchetypeId) ||
        spawned.strClientVisualId != spawned.strCombatObjectArchetypeId) return false;
    float age = 0.f;
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
        spawned.iServerTick, spawned.iSpawnTick, 30.f, age)) return false;
    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
    desc.iLevelIndex = m_Desc.iLayerLevelIndex;
    desc.strPlacementId = "maharaka.watergun.flight." + std::to_string(spawned.iCombatObjectId);
    desc.strEffectAssetId = Asset(spawned.strCombatObjectArchetypeId, "flight");
    desc.iSpawnTick = spawned.iSpawnTick;
    desc.fInitialSampleTimeSeconds = age;
    desc.bOwnerSustainedSourceLoops = true;
    // Source +X to gameplay +Z is stored once in the authored particle system.
    XMStoreFloat4x4(&desc.RootWorld, XMMatrixRotationY(XMConvertToRadians(spawned.fYawDegrees)) *
        XMMatrixTranslation(spawned.fPositionX, spawned.fPositionY, spawned.fPositionZ));
    EFFECT_WORLD_ROOT_HANDLE effect;
    if (!CEffectPresentationService::Spawn_LevelPlacement(desc, effect, status)) return false;
    handle.eKind = COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V1_LEVEL_ROOT;
    handle.iValue = effect.iValue;

    if (age < .2f && spawned.strCombatObjectArchetypeId != "maharaka.watergun.bomb")
    {
        desc.strPlacementId = "maharaka.watergun.start." + std::to_string(spawned.iCombatObjectId);
        desc.strEffectAssetId = "effect.maharaka.watergun.shot.start";
        desc.bOwnerSustainedSourceLoops = false;
        const float scale = spawned.strCombatObjectArchetypeId == "maharaka.watergun.single" ? .7f : 1.f;
        XMStoreFloat4x4(&desc.RootWorld, XMMatrixScaling(scale, scale, scale) *
            XMMatrixRotationY(XMConvertToRadians(spawned.fYawDegrees)) *
            XMMatrixTranslation(spawned.fPositionX, spawned.fPositionY, spawned.fPositionZ));
        EFFECT_WORLD_ROOT_HANDLE start;
        // An optional launch splash failure cannot orphan the admitted flight handle.
        std::string splashStatus;
        if (!CEffectPresentationService::Spawn_LevelPlacement(desc, start, splashStatus))
            m_strPendingPresentationFailure = splashStatus;
    }
    return true;
}

bool Client::CClientReplication::Apply_WaterGunImpact(
    const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event)
{
    using namespace LostArk::Shared;
    const auto* record = m_CombatObjectProjectionRuntime.Find(event.iCombatObjectId);
    if (m_Desc.iLayerLevelIndex != ETOUI(LEVEL::MAHARAKA) || !record ||
        record->iSourceNetEntityId != event.iSourceNetEntityId ||
        record->strCombatObjectArchetypeId != event.strCombatObjectArchetypeId ||
        record->Snapshot.PinnedDefinitionRevision != event.PinnedDefinitionRevision ||
        event.eKind != COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE ||
        event.strOwnerPatternId != "maharaka.watergun" ||
        event.strOwnerStageActionId != "maharaka.watergun.shot" ||
        event.strHitId != "maharaka.watergun.impact" || !event.iEventSequence || !event.iServerTick ||
        event.iRepeatIndex != 0u || !std::isfinite(event.fPositionX) ||
        !std::isfinite(event.fPositionY) || !std::isfinite(event.fPositionZ) ||
        !std::isfinite(event.fYawDegrees)) return false;
    if (record->bPresentationCompleted) return true;

    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
    desc.iLevelIndex = m_Desc.iLayerLevelIndex;
    desc.strPlacementId = "maharaka.watergun.impact." + std::to_string(event.iCombatObjectId);
    desc.strEffectAssetId = Asset(event.strCombatObjectArchetypeId, "hit");
    desc.iSpawnTick = event.iServerTick;
    XMStoreFloat4x4(&desc.RootWorld, XMMatrixRotationY(XMConvertToRadians(event.fYawDegrees)) *
        XMMatrixTranslation(event.fPositionX, event.fPositionY, event.fPositionZ));
    EFFECT_WORLD_ROOT_HANDLE effect;
    const bool spawned = CEffectPresentationService::Spawn_LevelPlacement(
        desc, effect, m_strPendingPresentationFailure);
    COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
    m_CombatObjectProjectionRuntime.Complete_Presentation(event.iCombatObjectId, sink);
    return spawned;
}

void Client::CClientReplication::Apply_WaterGunSpeed(
    const LostArk::Shared::PLAYER_SNAPSHOT& player, const std::uint32_t serverTick,
    const std::shared_ptr<CCharacter>& character)
{
    using namespace LostArk::Shared;
    std::uint32_t endTick = 0u;
    if (m_Desc.iLayerLevelIndex == ETOUI(LEVEL::MAHARAKA) && player.iCurrentHp && player.isWaterpangArmed)
        for (const auto& buff : player.ActiveBuffs)
            if (buff.iBuffId == MAHARAKA_WATERGUN_SPEED_BUFF_ID &&
                static_cast<std::int32_t>(buff.iEndTick - serverTick) > 0) endTick = buff.iEndTick;
    const auto npcView = m_WaterpangNpcPlayers.find(player.iNetEntityId);
    const auto npc = npcView != m_WaterpangNpcPlayers.end() ? npcView->second.npc.lock() : nullptr;
    const auto found = m_WaterGunSpeed.find(player.iNetEntityId);
    if (found != m_WaterGunSpeed.end())
    {
        if (found->second.endTick == endTick && found->second.owner.lock() == character &&
            found->second.npc.lock() == npc) return;
        if (found->second.worldRootHandle)
            CEffectPresentationService::Stop_WorldRoot({found->second.worldRootHandle});
        else CEffectPresentationService::Stop_CharacterOccurrence(found->second.owner.lock(), found->second.occurrence);
        m_WaterGunSpeed.erase(found);
    }
    if (!endTick) return;
    constexpr std::uint32_t durationTicks = 150u;
    const std::uint32_t remaining = endTick - serverTick;
    const float age = remaining < durationTicks ? float(durationTicks - remaining) / 30.f : 0.f;
    EFFECT_SPAWN_DESC desc;
    desc.strEffectAssetId = "effect.maharaka.watergun.e.speed";
    desc.pOwner = character;
    desc.strAnchorSlotId = "root";
    desc.strOccurrenceId = "maharaka.watergun.speed." + std::to_string(player.iNetEntityId) + "." + std::to_string(endTick);
    desc.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
    desc.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
    desc.iCueDurationMs = 5000u;
    desc.fInitialSampleTimeSeconds = age;
    WATERGUN_SPEED_PRESENTATION presentation{character, npc, endTick, desc.strOccurrenceId};
    if (npc)
    {
        if (!npc->Get_Transform()) return;
        EFFECT_LEVEL_PLACEMENT_SPAWN_DESC levelDesc;
        levelDesc.iLevelIndex = m_Desc.iLayerLevelIndex;
        levelDesc.strPlacementId = desc.strOccurrenceId;
        levelDesc.strEffectAssetId = desc.strEffectAssetId;
        levelDesc.iSpawnTick = serverTick;
        levelDesc.fInitialSampleTimeSeconds = age;
        levelDesc.pAnchorOwner = npc;
        levelDesc.RootWorld = *npc->Get_Transform()->Get_WorldMatrixPtr();
        EFFECT_WORLD_ROOT_HANDLE effect;
        if (!CEffectPresentationService::Spawn_LevelPlacement(levelDesc, effect, m_strPendingPresentationFailure)) return;
        presentation.worldRootHandle = effect.iValue;
    }
    else if (!CEffectPresentationService::Spawn(desc, m_strPendingPresentationFailure)) return;
    m_WaterGunSpeed.emplace(player.iNetEntityId, std::move(presentation));
}

void Client::CClientReplication::Update_WaterGunSpeedAnchors()
{
    for (const auto& [id, presentation] : m_WaterGunSpeed)
        if (presentation.worldRootHandle)
            if (const auto npc = presentation.npc.lock(); npc && npc->Get_Transform())
                CEffectPresentationService::Update_WorldRoot({presentation.worldRootHandle},
                    *npc->Get_Transform()->Get_WorldMatrixPtr());
}

void Client::CClientReplication::Clear_WaterGunSpeed(const LostArk::Shared::NET_ENTITY_ID entity)
{
    for (auto it = m_WaterGunSpeed.begin(); it != m_WaterGunSpeed.end();)
    {
        if (entity && it->first != entity) { ++it; continue; }
        if (it->second.worldRootHandle)
            CEffectPresentationService::Stop_WorldRoot({it->second.worldRootHandle});
        else CEffectPresentationService::Stop_CharacterOccurrence(it->second.owner.lock(), it->second.occurrence);
        it = m_WaterGunSpeed.erase(it);
    }
}
