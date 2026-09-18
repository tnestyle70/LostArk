#pragma once

#include <algorithm>
#include <cstdint>

namespace LostArk::Shared::KoukuTargetTracking
{
// Rotate-only tracking initially consumes ten times the old remaining-arc
// fraction. Movement-enabled tracking keeps its independent 180 deg/s limit.
inline constexpr std::uint32_t ROTATE_ONLY_RESPONSE_SCALE = 10u;

inline double RotateOnlyFraction(const std::uint64_t elapsedTicks,
    const std::uint64_t remainingTicks) noexcept
{
    if (!elapsedTicks || !remainingTicks) return 0.0;
    const auto elapsed = (std::min)(elapsedTicks, remainingTicks);
    const auto gain = (std::min<std::uint64_t>)(ROTATE_ONLY_RESPONSE_SCALE, remainingTicks);
    if (elapsed >= remainingTicks - gain + 1u) return 1.0;
    // Product of (remaining - gain) / remaining for every elapsed fixed tick,
    // telescoped to at most gain factors. Grouped updates equal individual ticks.
    double retained = 1.0;
    for (std::uint64_t index = 0u; index < gain; ++index)
        retained *= double(remainingTicks - elapsed - index) / double(remainingTicks - index);
    return 1.0 - retained;
}
}
