#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
    // Stable collider/logic/result identities are persisted; stage/projectile
    // addresses only describe their current owner in the existing skill document.
    struct CHARACTER_ACTION_COMBAT_ROW final
    {
        std::string strColliderId, strLogicId, strResultId, strResultKind;
        std::uint32_t iSkillId = 0u, iStageIndex = 0u;
        std::uint32_t iProjectileIndex = UINT32_MAX, iProjectileStartMs = 0u;
        std::uint32_t iTimeMs = 0u, iRepeatCount = 1u, iRepeatMs = 0u;
        std::uint32_t iAreaType = 1u, iMaxTargets = 0u, iPushMs = 0u;
        double fRange = 1.0, fAngleDegrees = 0.0, fWidth = 0.0;
        double fHeight = 1.5, fOffset = 0.0, fInner = 0.0, fPushRange = 0.0;
        bool bContact = false;
        bool operator==(const CHARACTER_ACTION_COMBAT_ROW&) const = default;
    };

    // Owns the existing HitShapes authoring file, never a second combat source.
    // Saving uses the exact loaded baseline and preserves unrelated JSON fields.
    class CCharacterActionCombatDocument final
    {
    public:
        bool Reload(std::string_view animationAssetId, std::string& outStatus);
        bool Save_Atomic(const std::vector<CHARACTER_ACTION_COMBAT_ROW>& rows,
            std::string& outStatus);
        const std::vector<CHARACTER_ACTION_COMBAT_ROW>& Get_Rows() const { return m_Rows; }
        const std::filesystem::path& Get_Path() const { return m_Path; }
    private:
        std::filesystem::path m_Path;
        std::string m_Baseline;
        std::vector<CHARACTER_ACTION_COMBAT_ROW> m_Rows;
    };
}
