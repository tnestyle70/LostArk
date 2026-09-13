#include "KoukuSaydonPreviewRootMotion.h"
#include <algorithm>
#include <cmath>

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
    const float verticalScale, std::string& status)
{
    if (m_SuppressionActive || !model || rows.empty() ||
        !std::isfinite(verticalScale) || verticalScale < 0.f)
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
    if (m_SuppressionActive || !model || m_Windows.empty()) return false;
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
    const double x = cycles * window.cycle.x + sampled.x - window.baseline.x;
    const double y = cycles * window.cycle.y + sampled.y - window.baseline.y +
        (window.maxAgeMs > 0.0 ? window.albionTakeoffUp * (std::clamp)(ageMs / window.maxAgeMs, 0.0, 1.0) : 0.0);
    const double z = cycles * window.cycle.z + sampled.z - window.baseline.z;
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
    m_RootIndex = UINT32_MAX;
}
