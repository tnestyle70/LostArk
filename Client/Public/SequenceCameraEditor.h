#pragma once
#include "EffectRecoveryCamera.h"
#include <functional>
#include <optional>

namespace Client
{
struct SEQUENCE_CAMERA_EDITOR_STATE final
{
    std::string selectedKeyId;
    float framesPerSecond = 30.f;
    std::uint32_t rangeStartMs = 0u, rangeEndMs = 0u;
    float3_t rangeOffset{};
    std::string validationStatus;
};
struct SEQUENCE_CAMERA_EDITOR_RESULT final
{
    bool changed = false;
    std::optional<std::uint32_t> seekLocalMs;
};
// Edits a caller-owned draft. The document owner validates/applies it and owns
// its clock, coordinate frame, window, Save and Publish operations.
// The caller verifies Free camera mode and returns Eye/LookAt/Up in the edited row's
// coordinate frame (including model-root conversion). Failure preserves the output pose.
using SEQUENCE_CAMERA_CAPTURE = std::function<bool(VALTAN_CINEMATIC_CAMERA_POSE&, std::string&)>;
class CSequenceCameraEditor final
{
public:
    static SEQUENCE_CAMERA_EDITOR_RESULT Render(EFFECT_CAMERA_ROW& row,
        SEQUENCE_CAMERA_EDITOR_STATE& state, std::uint32_t cursorLocalMs,
        const SEQUENCE_CAMERA_CAPTURE& captureFreeCamera = {});
    // Pose uses the row coordinate frame; time, lens and interpolation stay authored.
    static bool Set_KeyPose(EFFECT_CAMERA_ROW& row, const std::string& keyId,
        const VALTAN_CINEMATIC_CAMERA_POSE& pose, std::string& status);
    static bool Set_KeyTime(EFFECT_CAMERA_ROW& row, const std::string& keyId, std::uint32_t timeMs);
    static bool Add_Key(EFFECT_CAMERA_ROW& row, std::uint32_t timeMs, std::string& selectedKeyId);
    static bool Delete_Key(EFFECT_CAMERA_ROW& row, const std::string& keyId);
    static bool Offset_Range(EFFECT_CAMERA_ROW& row, std::uint32_t firstMs,
        std::uint32_t lastMs, const float3_t& offset);
};
}
