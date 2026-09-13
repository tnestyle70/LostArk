#pragma once
#include "KoukuSaydonCompositionDocument.h"
#include "Model.h"
#include <span>
namespace Client
{
class DATA_JSON_VALUE;
// Shared interpretation of a Logic's authored transition interval. The caller
// retains its semantic clip/clock; this only prepares the existing CModel pose.
class CKoukuSaydonAnimationBlend final
{
public:
    static bool Read_ProductWindows(const DATA_JSON_VALUE& value,
        std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW>& output, std::string& status);
    static bool Validate_ModelWindows(const Engine::CModel& model,
        std::span<const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> windows, std::string& status);
    static bool Sample_Pose(const Engine::CModel& model,
        std::span<const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> windows,
        double patternMs, Engine::CModel::ANIMATION_TRANSITION_POSE& output,
        bool& active, std::string& status);
};
}
