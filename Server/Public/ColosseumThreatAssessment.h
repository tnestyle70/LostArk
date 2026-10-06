#pragma once

#include "ServerCombatGeometry.h"
#include "Network/NetworkIds.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

namespace LostArk::Server
{
    struct SERVER_PLAYER;
    struct SERVER_COMBAT_OBJECT;
    class CGameplayCatalog;

    // A bounded, immutable observation for tactical choices. This predicts contact;
    // it never advances an action, consumes a hit ledger, or authorizes damage.
    class CColosseumThreatAssessment final
    {
    public:
        struct SAMPLE { float risk = 0.f; float firstImpactSeconds = 1.f; };
        static constexpr float HorizonSeconds = .8f;
        static constexpr std::size_t MaximumThreats = 256u;

        void Observe(const SERVER_PLAYER& observer,
            const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
            const std::vector<SERVER_COMBAT_OBJECT>& objects,
            const CGameplayCatalog& catalog, std::uint32_t tick);
        // Query a 150 ms occupation window beginning at the proposed arrival.
        [[nodiscard]] SAMPLE At(float x, float y, float z, float arrivalSeconds = 0.f) const;
        // Follow the segment at its travel speed, then occupy its endpoint briefly.
        // Times are relative to Observe; departure advances a cached observation.
        // A zero endpoint occupation joins consecutive motion segments without pauses.
        // Each predicted hit contributes once, even when several path samples meet it.
        [[nodiscard]] float Along(float startX, float startY, float startZ,
            float endX, float endY, float endZ, float travelSeconds,
            float departureSeconds = 0.f, float endpointOccupationSeconds = .15f) const;
        [[nodiscard]] std::size_t ThreatCount() const { return m_Count; }

    private:
        struct THREAT
        {
            SERVER_COMBAT_SHAPE_XZ shape;
            float x = 0.f, y = 0.f, z = 0.f, forwardX = 0.f, forwardZ = 1.f;
            float velocityX = 0.f, velocityZ = 0.f;
            float moveStart = 0.f, moveEnd = HorizonSeconds;
            float begin = 0.f, end = 0.f, height = 1.8f, weight = 1.f;
            bool testHeight = true;
        };
        void Add(THREAT threat);
        [[nodiscard]] bool Intersects(const THREAT& threat, float from, float to,
            float startX, float startY, float startZ, float endX, float endY, float endZ,
            float travelSeconds, float departureSeconds, float& firstImpact) const;
        std::array<THREAT, MaximumThreats> m_Threats{};
        std::size_t m_Count = 0u;
        float m_ProtectedUntil = 0.f;
    };
}
