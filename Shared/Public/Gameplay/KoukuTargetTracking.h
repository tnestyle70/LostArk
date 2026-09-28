#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>

namespace LostArk::Shared::KoukuTargetTracking
{
// Rotate-only tracking initially consumes ten times the old remaining-arc
// fraction. Movement-enabled tracking keeps its independent 180 deg/s limit.
inline constexpr std::uint32_t ROTATE_ONLY_RESPONSE_SCALE = 10u;

inline double RotateOnlyFraction(const std::uint64_t elapsedTicks,
    const std::uint64_t remainingTicks, const double responseScale = 1.0) noexcept
{
    if (!elapsedTicks || !remainingTicks || !std::isfinite(responseScale) || responseScale <= 0.0) return 0.0;
    if (responseScale != 1.0)
    {
        // Scale the same remaining-arc response per fixed tick. Product form
        // keeps grouped updates equal to individual ticks for a fixed target.
        double retained = 1.0;
        const auto elapsed = (std::min)(elapsedTicks, remainingTicks);
        for (std::uint64_t tick = 0u; tick < elapsed; ++tick)
        {
            const double remaining = static_cast<double>(remainingTicks - tick);
            const double fraction = (std::min)(1.0,
                double(ROTATE_ONLY_RESPONSE_SCALE) * responseScale / remaining);
            retained *= 1.0 - fraction;
            if (retained <= 0.0) return 1.0;
        }
        return 1.0 - retained;
    }
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

inline double RotateOnlyYaw(const double currentYaw, const double targetYaw,
    const std::uint64_t elapsedTicks, const std::uint64_t remainingTicks,
    const double responseScale = 1.0) noexcept
{
    if (!std::isfinite(currentYaw) || !std::isfinite(targetYaw)) return currentYaw;
    const double turn = std::remainder(targetYaw - currentYaw, 360.0);
    return std::remainder(currentYaw + turn *
        RotateOnlyFraction(elapsedTicks, remainingTicks, responseScale), 360.0);
}
}
