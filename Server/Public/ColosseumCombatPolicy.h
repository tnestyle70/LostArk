#pragma once

#include "ServerPlayer.h"

#include <cstdint>
#include <span>

namespace LostArk::Server
{
    // GameRoom/전투 오브젝트가 현재 호출을 위해 구성하는 경기 문맥이다. Players의 배열과 각 플레이어는 차용한다.
    // GameRoom의 지역 포인터 vector 및 원래 플레이어보다 오래 보관하지 않는다. 공격 갱신 시 현재 문맥을 다시 전달한다.
    struct SERVER_COLOSSEUM_COMBAT_CONTEXT final
    {
        LostArk::Shared::WORLD_ID eWorldId = LostArk::Shared::WORLD_ID::BERN;
        std::uint64_t iMatchId = 0u;
        bool bMatchActive = false;
        std::span<SERVER_PLAYER*> Players;
    };

    // 경기에서 줄인 실제 HP 풀과 구분하여 체력줄 피해량은 별도로 보관한 160줄 기준 HP를 사용한다.
    inline constexpr std::uint32_t COLOSSEUM_DAMAGE_REFERENCE_HEALTH_BARS = 160u;
    // PvP 이동 정책은 부호 있는 밀기 거리와 지속 시간을 기존 값의 1/10로 축소한다. 정수 기간은 소비자에서 올림한다.
    inline constexpr std::uint32_t COLOSSEUM_KNOCKBACK_DIVISOR = 10u;

    // 스킬/피격 경로가 같은 활성 경기의 유효한 팀/HP/참가 상태인지 확인한다.
    // Players span의 실제 포함 여부와 GRABBED는 이 함수에서 검사하지 않는다.
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

    // 주소와 두 identity가 서로 다르고 양쪽이 유효한 참가자이며 팀이 다를 때만 상대다.
    // 거리/도형 교차는 상위 명중 경로가 별도로 확인한다.
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

    // 미리 정산한 PvP 피해의 시전자와 경기 문맥을 공용 HP/반응 경로로 전달한다.
    // 공용 피해 함수는 두 포인터와 상대 조건을 다시 검사한다. 이 구조는 피해량이나 해당 객체의 소유권을 갖지 않는다.
    struct SERVER_COLOSSEUM_RESOLVED_DAMAGE final
    {
        const SERVER_COLOSSEUM_COMBAT_CONTEXT* pContext = nullptr;
        const SERVER_PLAYER* pSource = nullptr;
        bool bCritical = false;
    };
}
