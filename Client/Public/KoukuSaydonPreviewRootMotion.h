#pragma once

#include "KoukuSaydonCompositionDocument.h"
#include "Model.h"
#include <memory>
#include <span>

namespace Client
{
// A source-clock evaluator for the existing local preview actors. It does not
// advance animation, own world transforms, or participate in Server gameplay.
class CKoukuSaydonPreviewRootMotion final
{
public:
    ~CKoukuSaydonPreviewRootMotion();
    static bool Allows_AutomaticMotion(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
    bool Prepare(const std::shared_ptr<Engine::CModel>& model,
        const std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE>& rows,
        float verticalScale, std::string& status);
    bool Begin_Suppression();
    // Empty yaw span returns displacement in the model's actor frame. Otherwise
    // each row keeps the facing captured for its own stage, even after a seek.
    bool Sample_Displacement(double clockMs, std::span<const float> rowYawDegrees,
        float3_t& outDisplacement) const;
    void Reset();
private:
    struct WINDOW final
    {
        KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE row;
        double nativeMs = 0.0, sourceStart = 0.0, sourceEnd = 0.0, maxAgeMs = 0.0;
        float ticksPerSecond = 0.f, nativeTicks = 0.f;
        float3_t baseline{}, cycle{}, completed{};
        double albionTakeoffUp = 0.0;
    };
    bool Sample_Window(const WINDOW& window, double ageMs, float3_t& output) const;
    std::weak_ptr<Engine::CModel> m_Model;
    std::vector<WINDOW> m_Windows;
    uint32_t m_RootIndex = UINT32_MAX;
    float m_VerticalScale = 1.f;
    Engine::CModel::ROOT_MOTION_SUPPRESSION_STATE m_PreviousSuppression;
    bool m_SuppressionActive = false;
};
}
