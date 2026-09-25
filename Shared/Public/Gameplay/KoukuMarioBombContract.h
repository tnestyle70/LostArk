#pragma once

#include <array>
#include <cmath>
#include <cstdint>

namespace LostArk::Shared::KoukuMarioBomb
{
inline constexpr std::uint32_t INTERVAL_MS = 4000u;
inline constexpr float SPEED_MPS = 3.f;
inline constexpr float RADIUS_M = .36f;
inline constexpr float LOW_BOTTOM_M = .05f, HIGH_BOTTOM_M = .90f;
struct BINDING { std::uint8_t stage; const char* marker; const char* arrival; const char* exit; };
inline constexpr std::array<BINDING, 7u> BINDINGS{{
    {2u, "Mario2_Boom", "Mario2_go", "Mario2_Trigger_2"},
    {2u, "Mario2_Boom_1", "Mario2_Trigger_2", "Mario2_Trigger_4"},
    {2u, "Mario2_Boom_2", "Mario2_Trigger_4", "Mario2_Trigger_7"},
    {3u, "Mario3_Boom", "Mario3_Trigger_4", "Mario3_Trigger_5"},
    {3u, "Mario3_Boom_1", "Mario3_Trigger_4", "Mario3_Trigger_5"},
    {3u, "Mario3_Boom_2", "Mario3_Trigger_8", "Mario3_Trigger_10"},
    {4u, "Mario4_Boom", "Mario4_Tigger_6", "Mario4_Tigger_7"}
}};
inline std::uint32_t Seed(const char* marker) noexcept
{
    std::uint32_t result = 2166136261u;
    for (; *marker; ++marker) result = (result ^ static_cast<unsigned char>(*marker)) * 16777619u;
    return result;
}
inline float Bottom(const std::uint32_t seed, const std::int64_t birth) noexcept
{
    std::uint32_t random = seed ^ (static_cast<std::uint32_t>(birth) * 0x9e3779b9u);
    random ^= random << 13; random ^= random >> 17; random ^= random << 5;
    return random & 1u ? HIGH_BOTTOM_M : LOW_BOTTOM_M;
}
// Both consumers sample the Server clock and these bounded slot generations.
inline std::int64_t Birth(const double clockMs, const std::uint32_t phaseMs,
    const std::uint32_t slot, const std::uint32_t count) noexcept
{
    if (!count) return -1;
    const auto latest = static_cast<std::int64_t>(std::floor((clockMs - phaseMs) / INTERVAL_MS));
    return latest < slot ? -1 : latest - (latest - slot) % count;
}
}
