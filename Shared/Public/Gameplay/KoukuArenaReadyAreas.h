#pragma once

namespace LostArk::Shared
{
    // GateProgress/입장 명령과 Client 아레나가 같은 월드 m 영역을 사용한다. Prop300010의 사각 영역이다.
    // 아래 상수는 원본 mesh bounds * WModel preScale .01 * StartSize 18 기준이다.
    // 중심을 뺀 XZ를 저작 yaw -67.5 degree 기준으로 변환하고 높이로 아래 전투 바닥을 제외한다.
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

    // 3관문 도착 발판과 이동 가능한 울타리 범위다. 같은 XZ 아래 전투 바닥은 높이 조건으로 제외한다.
    inline bool Is_KoukuGate3EntryTerrace(const float x, const float y, const float z) noexcept
    {
        return x >= -30.f && x <= -5.f && y >= 23.5f && y <= 28.f && z >= 947.f && z <= 972.f;
    }

    // player.spawn.kouku.party01의 대기 발판이다. XZ 반경 10m와 Y 구간을 함께 만족해야 한다.
    inline bool Is_KoukuArenaStartArea(const float x, const float y, const float z) noexcept
    {
        const float dx = x - 3.29f, dz = z + 10.69f;
        return dx * dx + dz * dz <= 100.f && y >= 5.64f && y <= 11.64f;
    }
}
