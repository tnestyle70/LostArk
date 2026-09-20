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
        float verticalScale, std::string& status, float horizontalScale = 1.f);
    bool Begin_Suppression();
    // Empty yaw span returns displacement in the model's actor frame. Otherwise
    // each row keeps the facing captured for its own stage, even after a seek.
    bool Sample_Displacement(double clockMs, std::span<const float> rowYawDegrees,
        float3_t& outDisplacement) const;
    struct AIRBORNE_EVENT final
    {
        std::string occurrenceId, phase;
        std::string targetPositionPolicy = "APPEAR", selectedEffectGroupId;
        uint32_t clockMs = 0u, durationMs = 0u;
        double heightM = 0.0;
        float3_t destination{};
        size_t windowIndex = SIZE_MAX;
        double sourceUp = 0.0, remainingMinimumUp = 0.0;
        std::vector<std::pair<double, double>> landingPrefixUp;
    };
    // Includes absolute BOSS_TELEPORT_XZ source-clock rebases as well as Albion phases.
    bool Prepare_Airborne(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::string& status);
    const std::vector<AIRBORNE_EVENT>& Airborne_Events() const { return m_AirborneEvents; }
    // Target positions are pinned by the presentation owner in event order;
    // APPEAR policy samples on appearance; SELECT policy reuses its captured ground point.
    bool Sample_AirbornePosition(double clockMs, std::span<const float> rowYawDegrees,
        const float3_t& initialPosition, std::span<const float3_t> selectionPositions,
        float3_t& output) const;
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
    bool Sample_AirborneUp(size_t windowIndex, double clockMs, double& output) const;
    std::vector<AIRBORNE_EVENT> m_AirborneEvents;
    std::weak_ptr<Engine::CModel> m_Model;
    std::vector<WINDOW> m_Windows;
    uint32_t m_RootIndex = UINT32_MAX;
    float m_VerticalScale = 1.f;
    float m_HorizontalScale = 1.f;
    Engine::CModel::ROOT_MOTION_SUPPRESSION_STATE m_PreviousSuppression;
    bool m_SuppressionActive = false;
};
}
