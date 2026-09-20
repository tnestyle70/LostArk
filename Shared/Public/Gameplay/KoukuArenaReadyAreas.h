#pragma once

namespace LostArk::Shared
{
    // Authored Gate 3 arrival deck, including its walkable fence footprint.
    // Height separates the deck from the combat floor below the same XZ area.
    inline bool Is_KoukuGate3EntryTerrace(const float x, const float y, const float z) noexcept
    {
        return x >= -30.f && x <= -5.f && y >= 23.5f && y <= 28.f && z >= 947.f && z <= 972.f;
    }

    // Authored player.spawn.kakul.party01 and its initial waiting platform.
    inline bool Is_KoukuArenaStartArea(const float x, const float y, const float z) noexcept
    {
        const float dx = x - 3.29f, dz = z + 10.69f;
        return dx * dx + dz * dz <= 100.f && y >= 5.64f && y <= 11.64f;
    }
}
