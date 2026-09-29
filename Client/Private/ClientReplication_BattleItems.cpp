#include "ClientReplication.h"

#include "ActionPresentationTimeline.h"
#include "Character.h"
#include "Effect_PresentationService.h"
#include "Model.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    std::string BombEffect(const std::string_view archetype)
    {
        if (archetype == "battle.item.destruction_bomb") return "effect.world.item.destruction_bomb";
        if (archetype == "battle.item.whirlwind_grenade") return "effect.world.item.whirlwind_grenade";
        return {};
    }
}

bool Client::CClientReplication::Is_BattleItemProjectile(const std::string_view archetype)
{
    return archetype == "battle.item.destruction_bomb" || archetype == "battle.item.whirlwind_grenade";
}

bool Client::CClientReplication::Spawn_BattleItemProjectile(
    const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned,
    COMBAT_OBJECT_PRESENTATION_HANDLE& handle, std::string& status)
{
    if (spawned.strClientVisualId != spawned.strCombatObjectArchetypeId) return false;
    float age = 0.f;
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
        spawned.iServerTick, spawned.iSpawnTick, 30.f, age)) return false;
    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
    desc.iLevelIndex = m_Desc.iLayerLevelIndex;
    desc.strPlacementId = "battle.item.projectile." + std::to_string(spawned.iCombatObjectId);
    desc.strEffectAssetId = BombEffect(spawned.strCombatObjectArchetypeId) + ".flight";
    desc.iSpawnTick = spawned.iSpawnTick;
    desc.fInitialSampleTimeSeconds = age;
    desc.bOwnerSustainedSourceLoops = true;
    XMStoreFloat4x4(&desc.RootWorld, XMMatrixRotationY(XMConvertToRadians(spawned.fYawDegrees)) *
        XMMatrixTranslation(spawned.fPositionX, spawned.fPositionY, spawned.fPositionZ));
    EFFECT_WORLD_ROOT_HANDLE effect;
    if (!CEffectPresentationService::Spawn_LevelPlacement(desc, effect, status)) return false;
    handle.eKind = COMBAT_OBJECT_PRESENTATION_KIND::EFFECT_V1_LEVEL_ROOT;
    handle.iValue = effect.iValue;
    return true;
}

bool Client::CClientReplication::Apply_BattleItemImpact(
    const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event)
{
    using namespace LostArk::Shared;
    const auto* record = m_CombatObjectProjectionRuntime.Find(event.iCombatObjectId);
    if (!record || record->iSourceNetEntityId != event.iSourceNetEntityId ||
        record->strCombatObjectArchetypeId != event.strCombatObjectArchetypeId ||
        record->Snapshot.PinnedDefinitionRevision != event.PinnedDefinitionRevision ||
        event.eKind != COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE ||
        event.strHitId != "battle.item.impact" || !event.iEventSequence || !event.iServerTick ||
        event.iRepeatIndex != 0u || !std::isfinite(event.fPositionX) ||
        !std::isfinite(event.fPositionY) || !std::isfinite(event.fPositionZ) ||
        !std::isfinite(event.fYawDegrees)) return false;
    if (record->bPresentationCompleted) return true;

    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
    desc.iLevelIndex = m_Desc.iLayerLevelIndex;
    desc.strPlacementId = "battle.item.impact." + std::to_string(event.iCombatObjectId);
    desc.strEffectAssetId = BombEffect(event.strCombatObjectArchetypeId);
    desc.iSpawnTick = event.iServerTick;
    XMStoreFloat4x4(&desc.RootWorld, XMMatrixRotationY(XMConvertToRadians(event.fYawDegrees)) *
        XMMatrixTranslation(event.fPositionX, event.fPositionY, event.fPositionZ));
    EFFECT_WORLD_ROOT_HANDLE effect;
    const bool spawned = CEffectPresentationService::Spawn_LevelPlacement(
        desc, effect, m_strPendingPresentationFailure);
    // Even a rejected impact resource must not leave the now-exploded projectile visible.
    COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
    m_CombatObjectProjectionRuntime.Complete_Presentation(event.iCombatObjectId, sink);
    return spawned;
}

void Client::CClientReplication::Apply_BattleItemProtection(
    const LostArk::Shared::PLAYER_SNAPSHOT& player, const std::uint32_t serverTick,
    const std::shared_ptr<CCharacter>& character)
{
    for (const std::uint32_t buffId : {32282u, 33500u})
    {
        const auto key = std::make_pair(player.iNetEntityId, buffId);
        std::uint32_t endTick = 0u;
        if (player.iCurrentHp != 0u)
            for (const auto& buff : player.ActiveBuffs)
                if (buff.iBuffId == buffId && buff.iEndTick > serverTick) endTick = buff.iEndTick;
        auto found = m_BattleItemProtection.find(key);
        if (found != m_BattleItemProtection.end())
        {
            if (endTick == found->second.endTick && found->second.owner.lock() == character) continue;
            CEffectPresentationService::Stop_CharacterOccurrence(found->second.owner.lock(), found->second.occurrence);
            m_BattleItemProtection.erase(found);
        }
        if (!endTick) continue;
        const std::uint32_t startTick = endTick >= 90u ? endTick - 90u : 0u;
        const float age = serverTick > startTick ? static_cast<float>(serverTick - startTick) / 30.f : 0.f;
        EFFECT_SPAWN_DESC desc;
        desc.strEffectAssetId = buffId == 33500u ? "effect.world.item.time_stop" : "effect.world.item.holy_charm";
        desc.pOwner = character;
        desc.strAnchorSlotId = "root";
        desc.strOccurrenceId = "battle.item.buff." + std::to_string(player.iNetEntityId) + "." +
            std::to_string(buffId) + "." + std::to_string(endTick);
        desc.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
        desc.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
        desc.iCueDurationMs = 3000u;
        // A buff belongs to the recipient; it survives unrelated skill/action changes.
        desc.iActionStartTick = 0u;
        desc.fInitialSampleTimeSeconds = age;
        if (CEffectPresentationService::Spawn(desc, m_strPendingPresentationFailure))
            m_BattleItemProtection.emplace(key, BATTLE_ITEM_PROTECTION_PRESENTATION{
                character, endTick, desc.strOccurrenceId});
    }
}

void Client::CClientReplication::Clear_BattleItemProtection(const LostArk::Shared::NET_ENTITY_ID entity)
{
    for (auto it = m_BattleItemProtection.begin(); it != m_BattleItemProtection.end();)
    {
        if (entity != LostArk::Shared::INVALID_NET_ENTITY_ID && it->first.first != entity) { ++it; continue; }
        CEffectPresentationService::Stop_CharacterOccurrence(it->second.owner.lock(), it->second.occurrence);
        it = m_BattleItemProtection.erase(it);
    }
}

LostArk::Shared::NET_ENTITY_ID Client::CClientReplication::Find_ItemTargetPlayerFromRay(
    const float3_t& origin, const float3_t& direction) const
{
    using namespace LostArk::Shared;
    NET_ENTITY_ID target = INVALID_NET_ENTITY_ID;
    float nearest = (std::numeric_limits<float>::max)();
    const auto local = Get_LocalCharacter();
    for (const auto& player : m_Registry.Get_LivePlayers())
    {
        if (!player.pCharacter || player.pCharacter == local ||
            player.Record.eControlKind != PLAYER_CONTROL_KIND::HUMAN ||
            player.pCharacter->Is_WorldPresentationHidden() ||
            !m_PlayerHealth.Find(player.Record.iNetEntityId).iCurrentHp ||
            std::none_of(m_PartyRoster.Members.begin(), m_PartyRoster.Members.end(), [&](const auto& member) {
                return member.iNetEntityId == player.Record.iNetEntityId;
            })) continue;
        const auto model = player.pCharacter->Get_BodyModel();
        float4x4_t world; float distance = 0.f;
        if (model && player.pCharacter->Try_Get_PresentationRootMatrix(&world) &&
            model->Try_PickCurrentPose(world, origin, direction, distance) && distance < nearest)
        {
            target = player.Record.iNetEntityId;
            nearest = distance;
        }
    }
    return target;
}
