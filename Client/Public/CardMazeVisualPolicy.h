#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

namespace Client
{
    // The single translucent card-maze floor that must draw before world
    // sprites in BLEND. Both stable IDs must match; nothing else is promoted.
    inline bool IsCardMazeFloorReceiver(std::uint64_t placementId, std::string_view assetId)
    {
        return placementId == 10296705976280178153ull &&
            assetId == "MAP_3C514C107BAB_LV_OCN_FORGOTTENIS_PLANE01_SM_OVR_017DC7A6977C";
    }

    // The eight card-maze floor symbol groups that use bounded retry.
    inline bool IsCardMazeMarkGroup(std::string_view asset)
    {
        constexpr std::array<std::string_view, 8> ids = {
            "cardmaze.mark.heart", "cardmaze.mark.spade", "cardmaze.mark.club", "cardmaze.mark.diamond",
            "cardmaze.exit.heart", "cardmaze.exit.spade", "cardmaze.exit.club", "cardmaze.exit.diamond"
        };
        for (const auto id : ids) if (asset == id) return true;
        return false;
    }

    // Per mark occurrence: first attempt plus at most two retries, at least
    // one second apart. A new Effect cache generation restarts the budget.
    struct CARD_MAZE_MARK_RETRY final
    {
        std::uint64_t generation = (std::numeric_limits<std::uint64_t>::max)();
        std::uint64_t nextAttemptMs = 0;
        std::uint32_t attempts = 0;

        void ObserveGeneration(std::uint64_t value)
        {
            if (generation == value) return;
            generation = value;
            attempts = 0;
            nextAttemptMs = 0;
        }
        bool TryBegin(std::uint64_t nowMs)
        {
            if (attempts >= 3u || nowMs < nextAttemptMs) return false;
            ++attempts;
            Defer(nowMs);
            return true;
        }
        void Defer(std::uint64_t nowMs)
        {
            constexpr auto maximum = (std::numeric_limits<std::uint64_t>::max)();
            nextAttemptMs = nowMs > maximum - 1000u ? maximum : nowMs + 1000u;
        }
    };
}
