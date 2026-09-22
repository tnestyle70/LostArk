#pragma once

namespace LostArk::Shared
{
    // Prop300010: original square mesh bounds * WModel preScale .01 * StartSize 18.
    // Local XZ rectangle follows the authored -67.5 degree yaw; height rejects the floor below.
    inline bool Is_KoukuGate3EntryAura(const float x, const float y, const float z) noexcept
    {
        const float dx = x + 22.20617676f, dz = z - 954.5942993f;
        constexpr float cosine = 0.3826834324f, sine = -0.9238795325f;
        const float localX = dx * cosine - dz * sine;
        const float localZ = dx * sine + dz * cosine;
        return y >= 24.59f && y <= 27.59f &&
            localX >= -3.610384827f && localX <= 3.610384827f &&
            localZ >= -3.633995132f && localZ <= 3.633995132f;
    }

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
