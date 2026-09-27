#pragma once
#include "EffectRecoveryCamera.h"
#include <filesystem>

namespace Client
{
// Camera-only authoring transaction for the existing Product recovery document.
class CRecoveryCameraAuthoringSession final
{
public:
    bool Open(const std::string& effectId, std::string& status);
    bool Apply(const EFFECT_CAMERA_ROW& row, std::string& status);
    bool Save(std::string& status);
    bool Is_Dirty() const { return m_Dirty; }
    const std::string& EffectId() const { return m_EffectId; }
    const std::vector<EFFECT_CAMERA_ROW>& Rows() const { return m_Rows; }
    // Pure staging and an explicit path allow sandbox contract tests to exercise
    // the same writer without replacing Product files.
    static bool Merge(const DATA_JSON_VALUE& baseline, const std::vector<EFFECT_CAMERA_ROW>& rows,
        const DATA_JSON_VALUE& current, DATA_JSON_VALUE& merged, std::string& status);
    static bool Save_File(const std::filesystem::path& path, const std::string& effectId,
        const DATA_JSON_VALUE& baseline, const std::vector<EFFECT_CAMERA_ROW>& rows,
        EFFECT_RECOVERY_CAMERA_DOCUMENT& saved, std::string& status);
private:
    std::string m_EffectId;
    DATA_JSON_VALUE m_Baseline;
    std::vector<EFFECT_CAMERA_ROW> m_Rows;
    bool m_Dirty = false;
};
}
