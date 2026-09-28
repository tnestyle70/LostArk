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

// Source: Projectile/531200.loa SkillEffect 531201 grants SkillBuff 555010 (CombatEffect 5000, Key 34).
struct GUARD
{
    ESTHER_ID eEstherId;
    const char* pArchetypeId;
    std::uint32_t iGrantTimeMs;
    float fRadiusM;
    float fOffsetForwardM;
    std::uint32_t iDurationMs;
    std::int32_t iDamageTakenPercent;
};

inline constexpr GUARD GUARDS[] =
{
    { ESTHER_ID::BAHUNTUR, "NPC_59060", 1000u, 7.f, 4.2f, 30000u, -50 },
};

// Source: Projectile/531500.loa summons NPC 54050 at 2.0 s; its aura 555030 (700 cm, 1000 ms) carries 555032 (Immune 20)
// and 555034 -> SkillEffect 531513 (madness gauge 3708100, -10); 555031 ValueC 10000 ends it; 555033 heals 3500 on release.
struct ZONE
{
    ESTHER_ID eEstherId;
    const char* pArchetypeId;
    std::uint32_t iStartMs;
    std::uint32_t iDurationMs;
    float fRadiusM;
    std::uint32_t iPulseMs;
    std::uint32_t iMadnessDrainPercent;
    std::uint32_t iReleaseHealPercent;
};

inline constexpr ZONE ZONES[] =
{
    { ESTHER_ID::INANNA, "NPC_59620", 2000u, 10000u, 7.f, 1000u, 10u, 35u },
};
inline constexpr const char* GUARD_BLOCKED_DAMAGE_PROFILES[] =
{
    "damage.valtan.omnidirectional-wipe-130",
    "damage.valtan.magic-orb-failure",
};

[[nodiscard]] inline bool Same_Id(const char* left, const char* right) noexcept
{
    if (nullptr == left || nullptr == right)
        return false;
    while (*left && *left == *right) { ++left; ++right; }
    return *left == *right;
}

[[nodiscard]] inline const GUARD* Find_Guard(const ESTHER_ID estherId) noexcept
{
    for (const GUARD& guard : GUARDS)
        if (guard.eEstherId == estherId)
            return &guard;
    return nullptr;
}

[[nodiscard]] inline const GUARD* Find_GuardByArchetype(const char* pArchetypeId) noexcept
{
    for (const GUARD& guard : GUARDS)
        if (Same_Id(guard.pArchetypeId, pArchetypeId))
            return &guard;
    return nullptr;
}

[[nodiscard]] inline const ZONE* Find_Zone(const ESTHER_ID estherId) noexcept
{
    for (const ZONE& zone : ZONES)
        if (zone.eEstherId == estherId)
            return &zone;
    return nullptr;
}

[[nodiscard]] inline const ZONE* Find_ZoneByArchetype(const char* pArchetypeId) noexcept
{
    for (const ZONE& zone : ZONES)
        if (Same_Id(zone.pArchetypeId, pArchetypeId))
            return &zone;
    return nullptr;
}

[[nodiscard]] inline bool Is_GuardBlockedDamageProfile(const char* pDamageProfileId) noexcept
{
    for (const char* blocked : GUARD_BLOCKED_DAMAGE_PROFILES)
        if (Same_Id(blocked, pDamageProfileId))
            return true;
    return false;
}

[[nodiscard]] inline const DEFINITION* Find(const ESTHER_ID estherId) noexcept
{
    for (const DEFINITION& definition : DEFINITIONS)
        if (definition.eEstherId == estherId)
            return &definition;
    return nullptr;
}

[[nodiscard]] inline const DEFINITION* Find_ByArchetype(const char* pArchetypeId) noexcept
{
    for (const DEFINITION& definition : DEFINITIONS)
        if (Same_Id(definition.pArchetypeId, pArchetypeId))
            return &definition;
    return nullptr;
}
}
