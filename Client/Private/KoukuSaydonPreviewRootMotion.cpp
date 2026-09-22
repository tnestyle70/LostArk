#include "KoukuSaydonPreviewRootMotion.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace Client;

CKoukuSaydonPreviewRootMotion::~CKoukuSaydonPreviewRootMotion() { Reset(); }

bool CKoukuSaydonPreviewRootMotion::Allows_AutomaticMotion(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
    if (pattern.BossMotion) return false;
    for (const auto& box : pattern.LogicOccurrences)
    {
        if (!box.bEnabled) continue;
        const auto logic = std::find_if(document.Logics.begin(), document.Logics.end(),
            [&](const auto& value) { return value.strLogicId == box.strLogicId; });
        if (logic != document.Logics.end() && (logic->fBossChargeDistanceM > 0.0 ||
            logic->strTriggerKind == "REAL_GAZE_TELEPORT")) return false;
    }
    return true;
}

bool CKoukuSaydonPreviewRootMotion::Prepare(const std::shared_ptr<Engine::CModel>& model,
    const std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE>& rows,
    const float verticalScale, std::string& status, const float horizontalScale)
{
    if (m_SuppressionActive || !model ||
        !std::isfinite(verticalScale) || verticalScale < 0.f ||
        !std::isfinite(horizontalScale) || horizontalScale < 0.f || horizontalScale > 1.f)
    { status = "Root motion requires an idle prepared owner and finite source rows."; return false; }
    const auto root = model->Find_BoneIndex("b_root");
    if (root < 0) { status = "Animation root motion requires b_root in the selected model."; return false; }
    std::vector<WINDOW> staged;
    for (const auto& row : rows)
    {
        WINDOW window;
        window.row = row;
        uint32_t index = UINT32_MAX;
        for (uint32_t i = 0u; i < model->Get_NumAnimations(); ++i)
            if (const auto* name = model->Get_AnimationName(i); name && row.strRuntimeClip == name)
            {
                if (index != UINT32_MAX) { status = "Ambiguous root-motion clip: " + row.strRuntimeClip; return false; }
                index = i;
            }
        float cursor = 0.f, ticks = 0.f;
        window.ticksPerSecond = model->Get_AnimationTickPerSecond(index);
        if (index == UINT32_MAX || !model->Get_AnimationProgress(index, cursor, ticks) ||
            !std::isfinite(ticks) || ticks <= 0.f || !std::isfinite(window.ticksPerSecond) ||
            window.ticksPerSecond <= 0.f || !row.iPlayMs || !std::isfinite(row.fPlayRate) || row.fPlayRate <= 0.f)
        { status = "Invalid root-motion clip timing: " + row.strRuntimeClip; return false; }
        window.nativeTicks = ticks;
        window.nativeMs = ticks * 1000.0 / window.ticksPerSecond;
        double ignored = 0.0;
        if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(row.iSourceStartMs,
            row.iSourceEndMs, 0.0, row.fPlayRate, window.nativeMs,
            row.strEndPolicy == "LOOP_TO_WINDOW", ignored))
        { status = "Root-motion source range is outside the native clip: " + row.strRuntimeClip; return false; }
        window.sourceStart = (std::min)(double(row.iSourceStartMs), window.nativeMs);
        window.sourceEnd = row.iSourceEndMs ? (std::min)(double(row.iSourceEndMs), window.nativeMs) : window.nativeMs;
        window.maxAgeMs = row.iPlayMs;
        float3_t endpoint;
        if (!model->Sample_AnimationRootTranslation(row.strRuntimeClip.c_str(),
                (std::min)(window.nativeTicks, float(window.sourceStart * window.ticksPerSecond * .001)), uint32_t(root), 2, verticalScale, window.baseline) ||
            !model->Sample_AnimationRootTranslation(row.strRuntimeClip.c_str(),
                (std::min)(window.nativeTicks, float(window.sourceEnd * window.ticksPerSecond * .001)), uint32_t(root), 2, verticalScale, endpoint))
        { status = "Root motion cannot sample a fixed finite b_root frame: " + row.strRuntimeClip; return false; }
        window.cycle = {endpoint.x - window.baseline.x, endpoint.y - window.baseline.y, endpoint.z - window.baseline.z};
        staged.push_back(std::move(window));
    }
    // Rows arrive in the same deterministic order used by the pose consumer.
    // A later row's delay can already own the pose, so the previous row stops
    // contributing at that pose boundary, not at the later source-start clock.
    for (size_t i = 0u; i + 1u < staged.size(); ++i)
    {
        const auto boundary = staged[i + 1u].row.iPoseStartMs;
        if (staged[i].row.iStartOffsetMs > staged[i + 1u].row.iStartOffsetMs)
        { status = "Root-motion source rows must be sorted by their pose clock."; return false; }
        if (boundary != UINT32_MAX)
            staged[i].maxAgeMs = (std::min)(staged[i].maxAgeMs,
                (std::max)(0.0, double(boundary) - staged[i].row.iStartOffsetMs));
    }
    m_Model = model;
    m_RootIndex = uint32_t(root);
    m_VerticalScale = verticalScale;
    m_HorizontalScale = horizontalScale;
    for (auto& window : staged)
        if (!Sample_Window(window, window.maxAgeMs, window.completed))
        { m_Model.reset(); status = "Root-motion endpoint is unavailable: " + window.row.strRuntimeClip; return false; }
    // Albion's _03 is a fixed airborne pose, not an ascent. Supply its actor
    // ascent from the following landing run's measured net drop. This exact
    // source action is also adapted by the Server root-motion publisher.
    const auto isAlbionClip = [](const WINDOW& window, const char* clip)
        { return window.row.iSourceActionId == 4219903u && window.row.strRuntimeClip == clip; };
    for (size_t begin = 0u; begin < staged.size();)
    {
        if (!isAlbionClip(staged[begin], "rpct00_att_battle_24_03")) { ++begin; continue; }
        size_t landing = begin;
        double ascentMs = 0.0, sourceUp = 0.0;
        while (landing < staged.size() && isAlbionClip(staged[landing], "rpct00_att_battle_24_03"))
        {
            ascentMs += staged[landing].maxAgeMs;
            sourceUp += staged[landing].completed.y;
            ++landing;
        }
        size_t end = landing;
        double landingUp = 0.0;
        while (end < staged.size() && (isAlbionClip(staged[end], "rpct00_att_battle_24_04") ||
            isAlbionClip(staged[end], "rpct00_att_battle_24_05")))
            landingUp += staged[end++].completed.y;
        const double lift = (std::max)(0.0, -landingUp - sourceUp);
        if (!std::isfinite(lift) || lift > 100000.0 || !std::isfinite(ascentMs))
        { m_Model.reset(); status = "Albion takeoff exceeds the finite root-motion range."; return false; }
        if (end > landing && ascentMs > 0.0 && lift > 0.0)
            for (size_t i = begin; i < landing; ++i)
            {
                staged[i].albionTakeoffUp = lift * staged[i].maxAgeMs / ascentMs;
                staged[i].completed.y += float(staged[i].albionTakeoffUp);
            }
        begin = (std::max)(landing, end);
    }
    m_Windows = std::move(staged);
    status.clear();
    return true;
}

bool CKoukuSaydonPreviewRootMotion::Begin_Suppression()
{
    const auto model = m_Model.lock();
    if (m_SuppressionActive || !model) return false;
    if (m_Windows.empty()) return true; // Spatial-only preview has no native root to suppress.
    const auto previous = model->Capture_RootMotionSuppression();
    // All transferred XYZ lives in the actor; no second vertical jump in bones.
    if (!model->Configure_RootMotionSuppressionFromRest(m_RootIndex, 2, 0.f)) return false;
    m_PreviousSuppression = previous;
    m_SuppressionActive = true;
    return true;
}

bool CKoukuSaydonPreviewRootMotion::Sample_Window(const WINDOW& window,
    const double ageMs, float3_t& output) const
{
    const auto model = m_Model.lock();
    if (!model || !std::isfinite(ageMs)) return false;
    const double age = (std::clamp)(ageMs, 0.0, window.maxAgeMs) * window.row.fPlayRate;
    const double span = window.sourceEnd - window.sourceStart;
    const bool loop = window.row.strEndPolicy == "LOOP_TO_WINDOW";
    if (span < 0.0 || (loop && span <= 0.0)) return false;
    const double cycles = loop ? std::floor(age / span) : 0.0;
    const double local = loop ? age - cycles * span : (std::min)(age, span);
    float3_t sampled;
    if (!model->Sample_AnimationRootTranslation(window.row.strRuntimeClip.c_str(),
        (std::min)(window.nativeTicks, float((window.sourceStart + local) * window.ticksPerSecond * .001)), m_RootIndex,
        2, m_VerticalScale, sampled)) return false;
    const double horizontalScale = window.trackingOwnsHorizontal ? 0.0 : m_HorizontalScale;
    const double x = (cycles * window.cycle.x + sampled.x - window.baseline.x) * horizontalScale;
    const double y = cycles * window.cycle.y + sampled.y - window.baseline.y +
        (window.maxAgeMs > 0.0 ? window.albionTakeoffUp * (std::clamp)(ageMs / window.maxAgeMs, 0.0, 1.0) : 0.0);
    const double z = (cycles * window.cycle.z + sampled.z - window.baseline.z) * horizontalScale;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
        (std::max)({std::abs(x), std::abs(y), std::abs(z)}) > 100000.0) return false;
    output = {float(x), float(y), float(z)};
    return true;
}

bool CKoukuSaydonPreviewRootMotion::Sample_Displacement(const double clockMs,
    const std::span<const float> rowYawDegrees, float3_t& output) const
{
    if (!std::isfinite(clockMs) || (!rowYawDegrees.empty() && rowYawDegrees.size() != m_Windows.size())) return false;
    float3_t accumulated{};
    for (size_t i = 0u; i < m_Windows.size(); ++i)
    {
        const auto& window = m_Windows[i];
        const double age = clockMs - window.row.iStartOffsetMs;
        if (age <= 0.0) continue;
        float3_t delta;
        if (age >= window.maxAgeMs) delta = window.completed;
        else if (!Sample_Window(window, age, delta)) return false;
        const float radians = rowYawDegrees.empty() ? 0.f : XMConvertToRadians(rowYawDegrees[i]);
        if (!std::isfinite(radians)) return false;
        const float sine = std::sin(radians), cosine = std::cos(radians);
        accumulated.x += delta.x * cosine + delta.z * sine;
        accumulated.y += delta.y;
        accumulated.z += delta.z * cosine - delta.x * sine;
    }
    if (!std::isfinite(accumulated.x) || !std::isfinite(accumulated.y) || !std::isfinite(accumulated.z)) return false;
    output = accumulated;
    return true;
}

bool CKoukuSaydonPreviewRootMotion::Sample_AirborneUp(const size_t windowIndex,
    const double clockMs, double& output) const
{
    if (windowIndex >= m_Windows.size()) return false;
    const auto& window = m_Windows[windowIndex];
    float3_t delta;
    const double age = (std::clamp)(clockMs - window.row.iStartOffsetMs, 0.0, window.maxAgeMs);
    if (!Sample_Window(window, age, delta)) return false;
    // Native phase displacement only; the automatic _03 ascent is a separate policy.
    output = delta.y - (window.maxAgeMs > 0.0 ? window.albionTakeoffUp * age / window.maxAgeMs : 0.0);
    return std::isfinite(output);
}

bool CKoukuSaydonPreviewRootMotion::Prepare_Airborne(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::string& status)
{
    std::vector<AIRBORNE_EVENT> staged;
    for (const auto& box : pattern.LogicOccurrences)
    {
        if (!box.bEnabled) continue;
        const auto logic = std::find_if(document.Logics.begin(), document.Logics.end(),
            [&](const auto& row) { return row.strLogicId == box.strLogicId; });
        if (logic == document.Logics.end() || (logic->strTriggerKind != "ALBION_AIRBORNE" &&
            logic->strTriggerKind != "BOSS_TELEPORT_XZ" && logic->strTriggerKind != "BOSS_TELEPORT_GROUNDED" &&
            logic->strTriggerKind != "BOSS_TELEPORT_FACE_CENTER")) continue;
        AIRBORNE_EVENT event;
        event.occurrenceId = box.strOccurrenceId;
        event.phase = logic->strTriggerKind == "BOSS_TELEPORT_XZ" ? "TELEPORT_XZ" :
            logic->strTriggerKind == "BOSS_TELEPORT_GROUNDED" ? "TELEPORT_GROUNDED" :
            logic->strTriggerKind == "BOSS_TELEPORT_FACE_CENTER" ? "TELEPORT_FACE_CENTER" : logic->strAirbornePhase;
        event.clockMs = box.iStartMs; event.durationMs = logic->iAirborneDurationMs;
        event.targetPositionPolicy = logic->strAirborneTargetPositionPolicy;
        event.selectedEffectGroupId = logic->strSelectedEffectGroupId;
        event.heightM = logic->fAirborneHeightM;
        event.destination = {float(logic->TeleportPosition[0]), float(logic->TeleportPosition[1]), float(logic->TeleportPosition[2])};
        staged.push_back(std::move(event));
    }
    std::stable_sort(staged.begin(), staged.end(), [](const auto& a, const auto& b) {
        if (a.clockMs != b.clockMs) return a.clockMs < b.clockMs;
        if ((a.phase == "SELECT_PLAYER") != (b.phase == "SELECT_PLAYER")) return a.phase == "SELECT_PLAYER";
        return false; // Equal-clock triggers retain authored order, as on the Server.
    });
    bool jump = false;
    for (auto& event : staged)
    {
        if (event.phase == "TELEPORT_XZ" || event.phase == "TELEPORT_GROUNDED" || event.phase == "TELEPORT_FACE_CENTER")
        {
            if (!std::isfinite(event.destination.x) || !std::isfinite(event.destination.z) ||
                (event.phase != "TELEPORT_XZ" && !std::isfinite(event.destination.y)))
            { status = "Teleport preview requires finite destination XZ."; return false; }
            continue;
        }
        if (event.phase == "SELECT_PLAYER") continue;
        if (event.phase == "JUMP") jump = true;
        else if (!jump)
        { status = "Albion preview requires JUMP before height phases."; return false; }
        if (event.phase != "APPEAR_PLAYER" && event.phase != "SLAM") continue;
        for (size_t i = 0u; i < m_Windows.size(); ++i)
            if (m_Windows[i].row.iPoseStartMs <= event.clockMs) event.windowIndex = i;
        if (event.windowIndex == SIZE_MAX ||
            !Sample_AirborneUp(event.windowIndex, event.clockMs, event.sourceUp))
        { status = "Airborne phase needs a valid native animation pose owner."; return false; }
        event.remainingMinimumUp = event.sourceUp;
        event.landingPrefixUp.emplace_back(event.clockMs, event.sourceUp);
        const auto& window = m_Windows[event.windowIndex];
        const double endMs = window.row.iStartOffsetMs + window.maxAgeMs;
        // Native translation keys interpolate linearly. Include every source tick
        // plus both authored endpoints when finding the remaining landing minimum.
        const double stepMs = 1000.0 / (double(window.ticksPerSecond) * window.row.fPlayRate);
        const double ageAtEvent = (std::max)(0.0, double(event.clockMs) - window.row.iStartOffsetMs);
        double age = std::ceil((window.sourceStart + ageAtEvent * window.row.fPlayRate) * window.ticksPerSecond * .001) * stepMs
            - window.sourceStart / window.row.fPlayRate;
        for (double clock = window.row.iStartOffsetMs + age; clock < endMs; clock += stepMs)
        {
            double up = 0.0;
            if (!Sample_AirborneUp(event.windowIndex, clock, up)) return false;
            event.remainingMinimumUp = (std::min)(event.remainingMinimumUp, up);
            event.landingPrefixUp.emplace_back(clock, event.remainingMinimumUp);
        }
        double lastUp = 0.0;
        if (!Sample_AirborneUp(event.windowIndex, endMs, lastUp)) return false;
        event.remainingMinimumUp = (std::min)(event.remainingMinimumUp, lastUp);
        event.landingPrefixUp.emplace_back(endMs, event.remainingMinimumUp);
        if (event.phase == "SLAM" && event.sourceUp - event.remainingMinimumUp <= .000001)
        { status = "Albion SLAM needs a remaining downward native source curve."; return false; }
    }
    // A moving pursuit that owns a whole stage supplies that stage's XZ.
    // Keep the source Y and match the publisher's per-stage root-motion policy.
    auto stagedWindows = m_Windows;
    uint64_t stageStart = 0u;
    for (const auto& stage : pattern.Stages)
    {
        const uint64_t stageEnd = stageStart + stage.iDurationMs;
        const bool trackingOwnsHorizontal = std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto& box) {
            if (!box.bEnabled || box.iStartMs > stageStart || uint64_t(box.iStartMs) + box.iDurationMs < stageEnd) return false;
            const auto logic = std::find_if(document.Logics.begin(), document.Logics.end(),
                [&](const auto& row) { return row.strLogicId == box.strLogicId; });
            return logic != document.Logics.end() && logic->strJudgementKind == "BOSS_TRACK_TARGET" && logic->fFollowSpeedScale > 0.0;
        });
        for (auto& window : stagedWindows)
        {
            if (window.row.iPoseStartMs < stageStart || window.row.iPoseStartMs >= stageEnd ||
                window.trackingOwnsHorizontal == trackingOwnsHorizontal) continue;
            window.trackingOwnsHorizontal = trackingOwnsHorizontal;
            if (trackingOwnsHorizontal) window.completed.x = window.completed.z = 0.f;
            else
            {
                float3_t restored;
                if (!Sample_Window(window, window.maxAgeMs, restored))
                { status = "Root motion cannot restore the source XZ after pursuit editing."; return false; }
                window.completed.x = restored.x; window.completed.z = restored.z;
            }
        }
        stageStart = stageEnd;
    }
    m_Windows = std::move(stagedWindows);
    m_AirborneEvents = std::move(staged);
    status.clear();
    return true;
}

bool CKoukuSaydonPreviewRootMotion::Sample_AirbornePosition(const double clockMs,
    const std::span<const float> rowYawDegrees, const float3_t& initial,
    const std::span<const float3_t> selections, float3_t& output) const
{
    if (selections.size() != m_AirborneEvents.size() || !std::isfinite(clockMs)) return false;
    float3_t displacement;
    if (!Sample_Displacement(clockMs, rowYawDegrees, displacement)) return false;
    float3_t position{initial.x + displacement.x, initial.y + displacement.y, initial.z + displacement.z};
    float3_t selected = initial;
    double floor = initial.y, fullHeight = 0.0, phaseStartY = initial.y, offsetX = 0.0, offsetZ = 0.0;
    float3_t landingAnchor = initial, landingResumeDisplacement{};
    double landingResumeClock = (std::numeric_limits<double>::infinity)();
    const AIRBORNE_EVENT* active = nullptr;
    const auto heightAt = [&](const double clock, double& y) {
        if (!active)
        {
            float3_t delta;
            if (!Sample_Displacement(clock, rowYawDegrees, delta)) return false;
            y = initial.y + delta.y; return true;
        }
        const auto& phase = active->phase;
        if (phase == "TELEPORT_GROUNDED" || phase == "TELEPORT_FACE_CENTER")
        {
            // Local authoring uses the reference floor; product playback uses Server navigation.
            // A new row begins at that floor, while the current row keeps only its remaining Up.
            double baselineClock = active->clockMs;
            for (const auto& window : m_Windows)
                if (window.row.iPoseStartMs <= clock && window.row.iPoseStartMs > baselineClock)
                    baselineClock = window.row.iPoseStartMs;
            float3_t delta, baseline;
            if (!Sample_Displacement(clock, rowYawDegrees, delta) ||
                !Sample_Displacement(baselineClock, rowYawDegrees, baseline)) return false;
            y = floor + (std::max)(0.0, double(delta.y) - baseline.y);
        }
        else if (phase == "JUMP")
            y = active->durationMs == 0u ? floor + fullHeight :
                phaseStartY + (floor + fullHeight - phaseStartY) *
                (std::clamp)((clock - active->clockMs) / double(active->durationMs), 0.0, 1.0);
        else if (phase == "APPEAR_PLAYER" || phase == "SLAM")
        {
            if (phase == "SLAM" && clock >= landingResumeClock)
            {
                float3_t delta;
                if (!Sample_Displacement(clock, rowYawDegrees, delta)) return false;
                y = floor + delta.y - landingResumeDisplacement.y;
                return std::isfinite(y);
            }
            double up = 0.0;
            if (!Sample_AirborneUp(active->windowIndex, clock, up)) return false;
            if (phase == "APPEAR_PLAYER") y = floor + (std::max)(0.0, active->heightM + up);
            else
            {
                // Prefix minima reproduce the Server's monotonic landing even
                // when the authored root has a small rebound after touchdown.
                // Cached source ticks keep backward/forward seeks deterministic.
                const auto next = std::upper_bound(active->landingPrefixUp.begin(), active->landingPrefixUp.end(), clock,
                    [](double value, const auto& sample) { return value < sample.first; });
                if (next != active->landingPrefixUp.begin()) up = (std::min)(up, std::prev(next)->second);
                const double fraction = (std::clamp)((active->sourceUp - up) /
                    (active->sourceUp - active->remainingMinimumUp), 0.0, 1.0);
                y = floor + (phaseStartY - floor) * (1.0 - fraction);
            }
        }
        else y = floor + fullHeight;
        return std::isfinite(y);
    };
    for (size_t i = 0u; i < m_AirborneEvents.size(); ++i)
    {
        const auto& event = m_AirborneEvents[i];
        if (event.clockMs > clockMs) break;
        if (event.phase == "SELECT_PLAYER")
        { selected = selections[i]; continue; }
        if (event.phase == "TELEPORT_XZ" || event.phase == "TELEPORT_GROUNDED" || event.phase == "TELEPORT_FACE_CENTER")
        {
            float3_t atEvent;
            if (!Sample_Displacement(event.clockMs, rowYawDegrees, atEvent)) return false;
            offsetX = event.destination.x - initial.x - atEvent.x;
            offsetZ = event.destination.z - initial.z - atEvent.z;
            if (event.phase != "TELEPORT_XZ")
            { floor = event.destination.y; active = &event; }
            // Position keeps the source clock; the presentation owner applies face-center yaw.
            continue;
        }
        if (!heightAt(event.clockMs, phaseStartY)) return false;
        if (active && active->phase == "SLAM")
        {
            float3_t atEvent;
            if (!Sample_Displacement(event.clockMs, rowYawDegrees, atEvent)) return false;
            const auto& baseline = event.clockMs >= landingResumeClock ? landingResumeDisplacement : atEvent;
            offsetX = landingAnchor.x - initial.x - baseline.x;
            offsetZ = landingAnchor.z - initial.z - baseline.z;
        }
        if (event.phase == "JUMP") fullHeight = event.heightM;
        if (event.phase == "APPEAR_PLAYER") selected = selections[i];
        if (event.phase == "APPEAR_PLAYER" || event.phase == "CENTER")
        {
            const auto& destination = event.phase == "CENTER" ? event.destination : selected;
            float3_t atEvent;
            if (!Sample_Displacement(event.clockMs, rowYawDegrees, atEvent)) return false;
            offsetX = destination.x - initial.x - atEvent.x;
            offsetZ = destination.z - initial.z - atEvent.z;
            floor = destination.y;
        }
        if (event.phase == "SLAM")
        {
            float3_t atEvent;
            if (!Sample_Displacement(event.clockMs, rowYawDegrees, atEvent)) return false;
            landingAnchor = {initial.x + atEvent.x + float(offsetX), 0.f, initial.z + atEvent.z + float(offsetZ)};
            landingResumeClock = event.windowIndex + 1u < m_Windows.size() ?
                double(m_Windows[event.windowIndex + 1u].row.iPoseStartMs) : (std::numeric_limits<double>::infinity)();
            if (std::isfinite(landingResumeClock) && !Sample_Displacement(landingResumeClock, rowYawDegrees, landingResumeDisplacement)) return false;
        }
        active = &event;
    }
    double height = position.y;
    if (!heightAt(clockMs, height)) return false;
    position.x += float(offsetX); position.z += float(offsetZ); position.y = float(height);
    if (active && active->phase == "SLAM")
    {
        position.x = landingAnchor.x; position.z = landingAnchor.z;
        if (clockMs >= landingResumeClock)
        { position.x += displacement.x - landingResumeDisplacement.x; position.z += displacement.z - landingResumeDisplacement.z; }
    }
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return false;
    output = position;
    return true;
}

void CKoukuSaydonPreviewRootMotion::Reset()
{
    if (m_SuppressionActive)
        if (const auto model = m_Model.lock())
        {
            (void)model->Restore_RootMotionSuppression(m_PreviousSuppression);
            // Rebuild the same source pose with its former suppression policy.
            // This does not advance the timeline or change the selected clip.
            model->Update_Animation(0.f);
        }
    m_SuppressionActive = false;
    m_PreviousSuppression = {};
    m_Model.reset();
    m_Windows.clear();
    m_AirborneEvents.clear();
    m_RootIndex = UINT32_MAX;
}
