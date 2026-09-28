#pragma once

#include "Gameplay/CombatCollisionContract.h"
#include <cmath>
#include <cstdint>
#include <vector>

namespace LostArk::Shared::CombatObjectHitChain
{
    struct CONE_HIT final
    {
        float fOriginX = 0.f, fOriginZ = 0.f;
        float fForwardX = 0.f, fForwardZ = 1.f;
        float fLength = 0.f, fAngleDegrees = 0.f;
    };

    // Classify one owner hit once; callers own occurrence identity and the latch.
    // No overlap leaves the output empty and must not arm the group.
    inline bool Resolve_ConeDelays(const CONE_HIT& hit,
        const std::vector<CombatCollision::BODY_CIRCLE_XZ>& bodies,
        const std::uint32_t delayMs, std::vector<std::uint32_t>& outDelays)
    {
        outDelays.clear();
        if (bodies.empty() || delayMs == 0u || delayMs > 600000u ||
            !std::isfinite(hit.fOriginX) || !std::isfinite(hit.fOriginZ) ||
            !std::isfinite(hit.fForwardX) || !std::isfinite(hit.fForwardZ) ||
            !std::isfinite(hit.fLength) || !std::isfinite(hit.fAngleDegrees) ||
            hit.fLength <= 0.f || hit.fAngleDegrees <= 0.f || hit.fAngleDegrees > 360.f ||
            hit.fForwardX * hit.fForwardX + hit.fForwardZ * hit.fForwardZ < 0.000001f)
            return false;
        for (const auto& body : bodies)
            if (!CombatCollision::Is_Valid(body)) return false;
        bool anyHit = false;
        outDelays.reserve(bodies.size());
        for (const auto& body : bodies)
        {
            const bool intersects = CombatCollision::Circle_IntersectsCone(body,
                hit.fOriginX, hit.fOriginZ, hit.fForwardX, hit.fForwardZ,
                hit.fLength, hit.fAngleDegrees);
            anyHit = anyHit || intersects;
            outDelays.push_back(intersects ? 0u : delayMs);
        }
        if (!anyHit) outDelays.clear();
        return anyHit;
    }
}
