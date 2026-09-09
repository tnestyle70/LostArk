#pragma once

#include "DataJson.h"
#include "ValtanCinematicCameraController.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Client
{
struct EFFECT_CAMERA_ROW final
{
    std::string id, label, source = "PROJECT_TUNED";
    std::uint32_t startMs = 0u;
    bool muted = false, modelRelative = true, horizontalFov = false;
    VALTAN_CINEMATIC_CAMERA_CUE cue;
    std::vector<float3_t> upVectors;
};

struct EFFECT_RECOVERY_CAMERA_DOCUMENT final
{
    DATA_JSON_VALUE document;
    std::string asset, sequence;
    std::uint32_t version = 0u, durationMs = 0u;
    std::vector<EFFECT_CAMERA_ROW> rows;
    std::vector<std::string> localOnlyElementIds;
    bool exists = false;
};

// Shared value parsing/sampling. Camera ownership remains with each real caller.
class CEffectRecoveryCamera final
{
public:
    static bool Load(const std::string& effectId, EFFECT_RECOVERY_CAMERA_DOCUMENT& out, std::string& error);
    static bool Parse(const DATA_JSON_VALUE& document, bool required, std::vector<EFFECT_CAMERA_ROW>& out, std::string& error);
    static bool Validate(const std::vector<EFFECT_CAMERA_ROW>& rows, std::string& error);
    static bool Sample(const EFFECT_CAMERA_ROW& row, std::uint32_t clockMs, const float4x4_t& root,
        float aspect, VALTAN_CINEMATIC_CAMERA_POSE& pose, float3_t& up, std::string& error);
};
}
