#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Shared
{
// Spawn-relative attack meaning. Effect render elements never own this shape.
struct ATTACK_HIT_TEMPLATE final
{
    std::string strHitId, strTrigger = "TIMED", strShape = "CIRCLE";
    std::string strDamageKind = "MAX_HP_PERCENT", strDamageProfileId;
    std::uint32_t iAtMs = 0u, iEndMs = 0u, iRepeatCount = 1u, iRepeatIntervalMs = 0u, iDamagePercent = 10u;
    double fRadiusM = 1.0, fInnerRadiusM = 0.0, fLengthM = 0.0, fHalfWidthM = 0.0;
    double fAngleDegrees = 0.0, fOffsetForwardM = 0.0, fOffsetRightM = 0.0, fYawOffsetDegrees = 0.0;
    bool operator==(const ATTACK_HIT_TEMPLATE&) const = default;
};

inline bool Is_AttackStableId(const std::string& id)
{
    if (id.empty() || id.size() > 128u) return false;
    for (const unsigned char ch : id)
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') || ch == '_' || ch == '.' || ch == '-')) return false;
    return true;
}

inline bool Validate_AttackHitTemplates(const std::vector<ATTACK_HIT_TEMPLATE>& hits,
    const std::uint32_t lifetimeMs = 600000u)
{
    if (hits.size() > 32u) return false;
    for (std::size_t i = 0u; i < hits.size(); ++i)
    {
        const auto& h = hits[i];
        if (!Is_AttackStableId(h.strHitId) || h.iAtMs > 600000u || h.iEndMs > 600000u ||
            !h.iRepeatCount || h.iRepeatCount > 64u || h.iRepeatIntervalMs > 600000u ||
            (h.iRepeatCount > 1u && h.iRepeatIntervalMs < 34u)) return false;
        for (std::size_t j = 0u; j < i; ++j) if (hits[j].strHitId == h.strHitId) return false;
        for (const double v : {h.fRadiusM, h.fInnerRadiusM, h.fLengthM, h.fHalfWidthM})
            if (!std::isfinite(v) || v < 0.0 || v > 1000.0) return false;
        for (const double v : {h.fOffsetForwardM, h.fOffsetRightM})
            if (!std::isfinite(v) || std::abs(v) > 1000.0) return false;
        if (!std::isfinite(h.fAngleDegrees) || h.fAngleDegrees < 0.0 || h.fAngleDegrees > 360.0 ||
            !std::isfinite(h.fYawOffsetDegrees) || std::abs(h.fYawOffsetDegrees) > 360.0) return false;
        const std::uint64_t last = h.iAtMs + std::uint64_t(h.iRepeatCount - 1u) * h.iRepeatIntervalMs;
        if (h.strTrigger == "TIMED") { if (h.iEndMs || last > lifetimeMs) return false; }
        else if (h.strTrigger == "CONTACT") { if (h.iAtMs >= h.iEndMs || h.iEndMs > lifetimeMs) return false; }
        else return false;
        if (h.strShape == "CIRCLE") { if (h.fRadiusM <= 0.0 || h.fInnerRadiusM != 0.0) return false; }
        else if (h.strShape == "RING") { if (h.fInnerRadiusM <= 0.0 || h.fInnerRadiusM >= h.fRadiusM) return false; }
        else if (h.strShape == "BOX") { if (h.fLengthM <= 0.0 || h.fHalfWidthM <= 0.0) return false; }
        else if (h.strShape == "CONE") { if (h.fLengthM <= 0.0 || h.fAngleDegrees <= 0.0 || h.fInnerRadiusM >= h.fLengthM) return false; }
        else return false;
        if (h.strDamageKind == "PROFILE") { if (!Is_AttackStableId(h.strDamageProfileId) || h.iDamagePercent) return false; }
        else if (!h.strDamageProfileId.empty()) return false;
        else if (h.strDamageKind == "MAX_HP_PERCENT") { if (!h.iDamagePercent || h.iDamagePercent > 100u) return false; }
        else if (h.strDamageKind == "INSTANT_DEATH") { if (h.iDamagePercent) return false; }
        else return false;
    }
    return true;
}
}
