#pragma once

#include "ServerPlayer.h"

#include <cstdint>
#include <span>

namespace LostArk::Server
{
    // The room owns this view for one update. No arena context is retained by a
    // skill/projectile, so leaving a match immediately closes its damage authority.
    struct SERVER_COLOSSEUM_COMBAT_CONTEXT final
    {
        LostArk::Shared::WORLD_ID eWorldId = LostArk::Shared::WORLD_ID::BERN;
        std::uint64_t iMatchId = 0u;
        bool bMatchActive = false;
        std::span<SERVER_PLAYER*> Players;
    };

    // Damage keeps the original 160-bar reference independently of the match HP pool.
    inline constexpr std::uint32_t COLOSSEUM_DAMAGE_REFERENCE_HEALTH_BARS = 160u;
    // PvP keeps one tenth of the existing signed push distance and movement duration.
    inline constexpr std::uint32_t COLOSSEUM_KNOCKBACK_DIVISOR = 10u;

    [[nodiscard]] inline bool Is_ColosseumCombatParticipant(
        const SERVER_COLOSSEUM_COMBAT_CONTEXT& context, const SERVER_PLAYER& player) noexcept
    {
        return context.eWorldId == LostArk::Shared::WORLD_ID::COLOSSEUM &&
            context.bMatchActive && context.iMatchId != 0u && player.bColosseumParticipant &&
            player.iColosseumMatchId == context.iMatchId && player.iColosseumTeam < 2u &&
            player.iCurrentHp != 0u && player.isCombatReady &&
            player.eAction != LostArk::Shared::PLAYER_ACTION_STATE::DEAD &&
            player.eAction != LostArk::Shared::PLAYER_ACTION_STATE::FALLING;
    }

    [[nodiscard]] inline bool Is_ColosseumOpponent(
        const SERVER_COLOSSEUM_COMBAT_CONTEXT& context, const SERVER_PLAYER& attacker,
        const SERVER_PLAYER& target) noexcept
    {
        return &attacker != &target && attacker.iPlayerId != target.iPlayerId &&
            attacker.iNetEntityId != target.iNetEntityId &&
            Is_ColosseumCombatParticipant(context, attacker) &&
            Is_ColosseumCombatParticipant(context, target) &&
            attacker.iColosseumTeam != target.iColosseumTeam;
    }

    // Only the arena adapter passes this. The shared HP/CC commit rechecks the
    // context before accepting the already resolved player damage and attribution.
    struct SERVER_COLOSSEUM_RESOLVED_DAMAGE final
    {
        const SERVER_COLOSSEUM_COMBAT_CONTEXT* pContext = nullptr;
        const SERVER_PLAYER* pSource = nullptr;
        bool bCritical = false;
    };
}
