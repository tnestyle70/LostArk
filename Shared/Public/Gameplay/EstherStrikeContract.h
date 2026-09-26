#pragma once

#include "Network/PacketMessages.h"

#include <cstddef>
#include <cstdint>
#include <iterator>

namespace LostArk::Shared::EstherStrike
{
// Source: EFTable_SkillEffect rows reached from XMLData/Projectile/531000, 531100, 531300.loa.
inline constexpr std::uint8_t AREA_CIRCLE = 1u;
inline constexpr std::uint8_t AREA_FORWARD_BOX = 2u;
inline constexpr std::uint32_t DAMAGE_MULTIPLIER = 50u;

struct HIT
{
    std::uint32_t iTimeMs;
    std::uint8_t iAreaType;
    float fRangeM;
    float fWidthM;
    float fOffsetForwardM;
    float fOffsetRightM;
    std::uint32_t iDamageMin;
    std::uint32_t iDamageMax;
};

struct DEFINITION
{
    ESTHER_ID eEstherId;
    const char* pArchetypeId;
    SKILL_ID iSkillId;
    const HIT* pHits;
    std::size_t iHitCount;
};

inline constexpr HIT SILLIAN_HITS[] =
{
    { 3100u, AREA_FORWARD_BOX, 20.f, 8.f, 1.2f, 0.f, 275625u, 281250u },
    { 4200u, AREA_FORWARD_BOX, 20.f, 8.f, 1.2f, 0.f, 643125u, 656250u },
};
inline constexpr HIT WEI_HITS[] =
{
    { 1900u, AREA_CIRCLE, 4.f, 0.f, 2.5f, 1.f, 147000u, 150000u },
    { 3100u, AREA_CIRCLE, 4.f, 0.f, 2.5f, -1.f, 147000u, 150000u },
    { 4900u, AREA_CIRCLE, 4.f, 0.f, 3.f, 0.f, 220500u, 225000u },
    { 5800u, AREA_CIRCLE, 5.5f, 0.f, 3.f, 0.f, 343000u, 350000u },
};
inline constexpr HIT NINAV_HITS[] =
{
    { 3500u, AREA_FORWARD_BOX, 20.f, 8.f, 1.2f, 0.f, 1453250u, 1482910u },
};

inline constexpr DEFINITION DEFINITIONS[] =
{
    { ESTHER_ID::SILLIAN, "NPC_59030", 531000u, SILLIAN_HITS, std::size(SILLIAN_HITS) },
    { ESTHER_ID::WEI, "NPC_58700", 532000u, WEI_HITS, std::size(WEI_HITS) },
    { ESTHER_ID::NINAV, "NPC_59504", 534000u, NINAV_HITS, std::size(NINAV_HITS) },
};

static_assert(std::size(SILLIAN_HITS) <= 32u && std::size(WEI_HITS) <= 32u && std::size(NINAV_HITS) <= 32u);

[[nodiscard]] inline const DEFINITION* Find(const ESTHER_ID estherId) noexcept
{
    for (const DEFINITION& definition : DEFINITIONS)
        if (definition.eEstherId == estherId)
            return &definition;
    return nullptr;
}

[[nodiscard]] inline const DEFINITION* Find_ByArchetype(const char* pArchetypeId) noexcept
{
    if (nullptr == pArchetypeId)
        return nullptr;
    for (const DEFINITION& definition : DEFINITIONS)
    {
        const char* left = definition.pArchetypeId;
        const char* right = pArchetypeId;
        while (*left && *left == *right) { ++left; ++right; }
        if (*left == *right)
            return &definition;
    }
    return nullptr;
}
}
