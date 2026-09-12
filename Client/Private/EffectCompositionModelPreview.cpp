#include "EffectCompositionModelPreview.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "Npc.h"
#include "KoukuSaydonPresentationPlayer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <set>

namespace
{
using namespace Client;
constexpr std::uint32_t MAX_TIME_MS = 600000u;

bool Text(const DATA_JSON_VALUE& source, const char* key, std::string& out)
{
    const auto* value = source.Find(key);
    if (!value || !value->Is_String()) return false;
    out = value->Get_String();
    return true;
}

bool Unsigned(const DATA_JSON_VALUE& source, const char* key, std::uint32_t& out,
    std::uint32_t maximum = MAX_TIME_MS)
{
    const auto* value = source.Find(key);
    if (!value || !value->Is_Number()) return false;
    const double number = value->Get_Number();
    if (!std::isfinite(number) || number < 0 || number > maximum || std::floor(number) != number) return false;
    out = static_cast<std::uint32_t>(number);
    return true;
}

void Problem(std::string& status, const std::string& message)
{
    if (!status.empty()) status += " ";
    status += message;
}

// This fallback is a display projection, never a writable Composition document.
// An unrelated Logic/Effect catalog error must not erase the saved model rows.
bool Read_ModelProjection(const std::string& bytes,
    KOUKU_SAYDON_COMPOSITION_DOCUMENT& out, std::string& status)
{
    DATA_JSON_VALUE root;
    if (!CDataJson::Parse(bytes, root, status) || !root.Is_Object()) return false;
    std::string schema;
    KOUKU_SAYDON_COMPOSITION_DOCUMENT staged;
    const auto* patterns = root.Find("patterns");
    if (!Text(root, "schema", schema) || schema != "lostark.kouku-saydon-composition" ||
        !Unsigned(root, "formatVersion", staged.iFormatVersion, 3u) || staged.iFormatVersion != 3u ||
        !Unsigned(root, "revision", staged.iRevision, (std::numeric_limits<std::uint32_t>::max)() - 1u) || !staged.iRevision ||
        !Unsigned(root, "fixedTickHz", staged.iFixedTickHz, 30u) || staged.iFixedTickHz != 30u ||
        !patterns || !patterns->Is_Array() || patterns->Get_Array().size() > 4096u)
    { status = "Model reference requires a valid v3 Composition header and patterns array."; return false; }
    Text(root, "compositionId", staged.strCompositionId);
    Text(root, "areaId", staged.strAreaId);
    std::set<std::string> ids;
    for (const auto& value : patterns->Get_Array())
    {
        auto& pattern = staged.Patterns.emplace_back();
        Text(value, "patternId", pattern.strPatternId);
        Text(value, "displayName", pattern.strDisplayName);
        Text(value, "authoringStatus", pattern.strAuthoringStatus);
        Text(value, "actorProfileId", pattern.strActorProfileId);
        pattern.strGateId.clear();
        Text(value, "gateId", pattern.strGateId);
        if (value.Find("folderId") && (!Text(value, "folderId", pattern.strFolderId) || pattern.strFolderId.empty()))
            Problem(pattern.strLoadError, "Invalid saved parent identity.");
        Text(value, "targetBossPlacementId", pattern.strTargetBossPlacementId);
        if (pattern.strPatternId.empty() || !ids.insert(pattern.strPatternId).second ||
            !CKoukuSaydonCompositionDocument::Is_KnownGate(pattern.strGateId) ||
            CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(pattern.strTargetBossPlacementId) != pattern.strActorProfileId ||
            CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(pattern.strGateId, pattern.strActorProfileId) != pattern.strTargetBossPlacementId)
            Problem(pattern.strLoadError, "Invalid model reference identity or target.");
        if (const auto* yaw = value.Find("resetBossYawDegrees"); yaw && yaw->Is_Number() && std::isfinite(yaw->Get_Number()))
            pattern.ResetBossYawDegrees = yaw->Get_Number();
        const auto* stages = value.Find("stages");
        if (!stages || !stages->Is_Array() || stages->Get_Array().size() > 1024u)
        { Problem(pattern.strLoadError, "Missing or invalid stages."); continue; }
        std::uint64_t duration = 0u;
        for (const auto& stageValue : stages->Get_Array())
        {
            auto& stage = pattern.Stages.emplace_back();
            Text(stageValue, "stageId", stage.strStageId);
            Text(stageValue, "actionId", stage.strActionId);
            Text(stageValue, "stageKind", stage.strStageKind);
            if (stage.strStageId.empty() || !Unsigned(stageValue, "durationMs", stage.iDurationMs) || !stage.iDurationMs)
                Problem(pattern.strLoadError, "Invalid stage identity or duration.");
            duration += stage.iDurationMs;
            const auto* clips = stageValue.Find("animationOccurrences");
            if (!clips || !clips->Is_Array() || clips->Get_Array().size() > 4096u)
            { Problem(pattern.strLoadError, "Missing or invalid animationOccurrences."); continue; }
            for (const auto& clipValue : clips->Get_Array())
            {
                auto& clip = stage.AnimationOccurrences.emplace_back();
                Text(clipValue, "occurrenceId", clip.strOccurrenceId);
                Text(clipValue, "profileId", clip.strProfileId);
                Text(clipValue, "runtimeClip", clip.strRuntimeClip);
                Text(clipValue, "sourceStageId", clip.strSourceStageId);
                Text(clipValue, "sourceSlotId", clip.strSourceSlotId);
                Text(clipValue, "referenceRevision", clip.strReferenceRevision);
                Text(clipValue, "endPolicy", clip.strEndPolicy);
                Unsigned(clipValue, "sourceActionId", clip.iSourceActionId, (std::numeric_limits<std::uint32_t>::max)());
                const auto* rate = clipValue.Find("playRate");
                const bool valid = Unsigned(clipValue, "startOffsetMs", clip.iStartOffsetMs) &&
                    Unsigned(clipValue, "sourceStartMs", clip.iSourceStartMs) &&
                    Unsigned(clipValue, "playMs", clip.iPlayMs) && clip.iPlayMs &&
                    rate && rate->Is_Number() && std::isfinite(rate->Get_Number()) &&
                    rate->Get_Number() >= .01 && rate->Get_Number() <= 16. &&
                    std::uint64_t(clip.iStartOffsetMs) + clip.iPlayMs <= stage.iDurationMs &&
                    !clip.strOccurrenceId.empty() && !clip.strRuntimeClip.empty() &&
                    CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(clip.strProfileId) == pattern.strActorProfileId &&
                    (clip.strEndPolicy == "EXACT" || clip.strEndPolicy == "HOLD_LAST_POSE" || clip.strEndPolicy == "LOOP_TO_WINDOW");
                if (rate && rate->Is_Number()) clip.fPlayRate = static_cast<float>(rate->Get_Number());
                if (!valid) Problem(pattern.strLoadError, "Invalid saved clip: " + clip.strRuntimeClip + ".");
            }
        }
        if (duration > MAX_TIME_MS) Problem(pattern.strLoadError, "Pattern exceeds 600 seconds.");
    }
    if (const auto* folders = root.Find("folders"); folders && folders->Is_Array())
        for (const auto& value : folders->Get_Array())
        {
            auto& folder = staged.Folders.emplace_back();
            Text(value, "folderId", folder.strFolderId);
            Text(value, "gateId", folder.strGateId);
            Text(value, "displayName", folder.strDisplayName);
            if (folder.strFolderId.empty() || !CKoukuSaydonCompositionDocument::Is_KnownGate(folder.strGateId))
                folder.strLoadError = "Invalid saved folder identity.";
        }
    for (auto& pattern : staged.Patterns)
        if (!pattern.strFolderId.empty() && std::none_of(staged.Folders.begin(), staged.Folders.end(),
            [&](const auto& folder) { return folder.strFolderId == pattern.strFolderId && folder.strGateId == pattern.strGateId && folder.strLoadError.empty(); }))
            Problem(pattern.strLoadError, "Saved parent is missing or belongs to another Gate.");
    if (const auto* bundles = root.Find("bundles"); bundles && bundles->Is_Array())
        for (const auto& value : bundles->Get_Array())
        {
            auto& bundle = staged.Bundles.emplace_back();
            Text(value, "bundleId", bundle.strBundleId);
            Text(value, "gateId", bundle.strGateId);
            Text(value, "folderId", bundle.strFolderId);
            Text(value, "displayName", bundle.strDisplayName);
            Text(value, "authoringStatus", bundle.strAuthoringStatus);
            if (bundle.strBundleId.empty() || !CKoukuSaydonCompositionDocument::Is_KnownGate(bundle.strGateId))
                bundle.strLoadError = "Invalid saved bundle identity.";
            const auto* members = value.Find("members");
            if (!members || !members->Is_Array())
            { Problem(bundle.strLoadError, "Missing members array."); continue; }
            for (const auto& memberValue : members->Get_Array())
            {
                auto& member = bundle.Members.emplace_back();
                Text(memberValue, "memberId", member.strMemberId);
                Text(memberValue, "patternId", member.strPatternId);
                if (!Unsigned(memberValue, "startOffsetMs", member.iStartOffsetMs) || member.strMemberId.empty() || member.strPatternId.empty())
                    Problem(bundle.strLoadError, "Invalid saved member identity or offset.");
            }
        }
    out = std::move(staged);
    return true;
}
}

void Client::CEffectCompositionModelPreview::Set_Player(CKoukuSaydonPresentationPlayer* player)
{
    if (player == m_Player) return;
    Stop();
    m_Player = player;
}

bool Client::CEffectCompositionModelPreview::Reload()
{
    const auto path = CKoukuSaydonCompositionDocument::Resolve_Path();
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error || size == 0u || size > 16u * 1024u * 1024u)
    { m_Status = "Model reference source unavailable; previous rows preserved: " + path.string(); return false; }
    std::ifstream input(path, std::ios::binary);
    std::string bytes(static_cast<std::size_t>(size), '\0');
    if (!input.read(bytes.data(), static_cast<std::streamsize>(bytes.size())))
    { m_Status = "Model reference read failed; previous rows preserved."; return false; }
    KOUKU_SAYDON_COMPOSITION_DOCUMENT staged;
    std::string status;
    bool projected = !CKoukuSaydonCompositionDocument::Parse_Text(bytes, staged, status);
    projected |= std::any_of(staged.Patterns.begin(), staged.Patterns.end(),
        [](const auto& row) { return !row.strLoadError.empty(); });
    projected |= std::any_of(staged.Bundles.begin(), staged.Bundles.end(),
        [](const auto& row) { return !row.strLoadError.empty(); });
    projected |= std::any_of(staged.Folders.begin(), staged.Folders.end(),
        [](const auto& row) { return !row.strLoadError.empty(); });
    if (projected)
    {
        const std::string authoringStatus = status;
        if (!Read_ModelProjection(bytes, staged, status))
        { m_Status = "Model reference read failed; previous rows preserved: " + status; return false; }
        status = "Read-only model projection; unrelated authoring validation is excluded. " + authoringStatus;
    }
    Stop();
    m_Document = std::move(staged);
    m_Loaded = true;
    Build_Rows();
    m_Status = "Saved Composition rev " + std::to_string(m_Document.iRevision) + ": " +
        std::to_string(m_Document.Patterns.size()) + " patterns. " + (projected ? status : "Model rows do not require Product publish.");
    return true;
}

bool Client::CEffectCompositionModelPreview::Select_SourceEffect(const EFFECT_DOCUMENT_DESC& effect)
{
    if (!effect.SourceModelPreview)
    { m_Status = "This Effect has no saved source model preview."; return false; }
    const auto& source = *effect.SourceModelPreview;
    if (!CEffectDocumentCodec::Validate(effect, m_Status)) return false;
    if (!CKoukuSaydonCompositionDocument::Is_KnownGate(source.strGateId) ||
        CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(source.strTargetBossPlacementId) != source.strActorProfileId ||
        CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(source.strGateId, source.strActorProfileId) != source.strTargetBossPlacementId)
    { m_Status = "Source model preview actor and Gate do not match their saved placement."; return false; }
    KOUKU_SAYDON_COMPOSITION_DOCUMENT staged;
    staged.iRevision = 1u;
    KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
    pattern.strPatternId = "effect.model." + effect.strEffectAssetId;
    pattern.strDisplayName = effect.strDisplayName;
    pattern.strGateId = source.strGateId;
    pattern.strActorProfileId = source.strActorProfileId;
    pattern.strTargetBossPlacementId = source.strTargetBossPlacementId;
    KOUKU_SAYDON_COMPOSITION_STAGE stage;
    stage.strStageId = "source.animation";
    for (const auto& animation : source.Animations)
    {
        KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE clip;
        clip.strOccurrenceId = "source.animation." + std::to_string(stage.AnimationOccurrences.size());
        clip.strRuntimeClip = animation.strRuntimeClip;
        clip.strProfileId = source.strActorProfileId;
        clip.iStartOffsetMs = animation.iStartOffsetMs;
        clip.iSourceStartMs = animation.iSourceStartMs;
        clip.iPlayMs = animation.iPlayMs;
        clip.fPlayRate = animation.fPlayRate;
        clip.strEndPolicy = animation.strEndPolicy;
        stage.iDurationMs = (std::max)(stage.iDurationMs, clip.iStartOffsetMs + clip.iPlayMs);
        stage.AnimationOccurrences.push_back(std::move(clip));
    }
    pattern.Stages.push_back(std::move(stage));
    const auto selected = pattern.strPatternId;
    staged.Patterns.push_back(std::move(pattern));
    Stop();
    m_Document = std::move(staged); m_SelectedId = selected; m_IsBundle = false; m_Loaded = true;
    Build_Rows();
    m_Status = "Loaded this Effect's source model and animation windows.";
    return true;
}

bool Client::CEffectCompositionModelPreview::Select_Pattern(const std::string& patternId)
{
    if (std::none_of(m_Document.Patterns.begin(), m_Document.Patterns.end(),
        [&](const auto& row) { return row.strPatternId == patternId; }))
    { m_Status = "Saved Pattern is missing; previous selection preserved."; return false; }
    Stop(); m_SelectedId = patternId; m_IsBundle = false; Build_Rows(); return true;
}

bool Client::CEffectCompositionModelPreview::Select_Bundle(const std::string& bundleId)
{
    if (std::none_of(m_Document.Bundles.begin(), m_Document.Bundles.end(),
        [&](const auto& row) { return row.strBundleId == bundleId; }))
    { m_Status = "Saved Bundle is missing; previous selection preserved."; return false; }
    Stop(); m_SelectedId = bundleId; m_IsBundle = true; Build_Rows(); return true;
}

void Client::CEffectCompositionModelPreview::Clear_Selection()
{
    Stop(); m_SelectedId.clear(); m_IsBundle = false; Build_Rows();
    m_Status = "No model reference. Effect editing and playback remain independent.";
}

void Client::CEffectCompositionModelPreview::Append_Actor(
    const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern, const std::string& patternId,
    const std::string& memberId, const std::uint32_t offsetMs)
{
    auto& actor = m_Actors.emplace_back();
    actor.memberId = memberId; actor.patternId = patternId; actor.authoredOffsetMs = offsetMs;
    const auto ticks = (std::uint64_t(offsetMs) * 30u + 999u) / 1000u;
    actor.effectiveOffsetMs = double(ticks) * 1000.0 / 30.0;
    if (!pattern) { actor.status = "Missing saved Pattern: " + patternId; return; }
    actor.displayName = pattern->strDisplayName;
    actor.actorProfileId = pattern->strActorProfileId;
    actor.targetPlacementId = pattern->strTargetBossPlacementId;
    actor.status = pattern->strLoadError;
    std::uint64_t stageStart = 0u;
    const auto firstRow = m_Rows.size();
    for (const auto& stage : pattern->Stages)
    {
        for (const auto& clip : stage.AnimationOccurrences)
        {
            auto& row = m_Rows.emplace_back();
            row.memberId = memberId; row.patternId = patternId; row.stageId = stage.strStageId;
            row.occurrenceId = clip.strOccurrenceId; row.runtimeClip = clip.strRuntimeClip;
            row.profileId = clip.strProfileId; row.endPolicy = clip.strEndPolicy;
            row.startMs = actor.effectiveOffsetMs + double(stageStart) + clip.iStartOffsetMs;
            row.durationMs = clip.iPlayMs; row.sourceStartMs = clip.iSourceStartMs; row.playRate = clip.fPlayRate;
            row.status = actor.status;
            if (clip.strRuntimeClip.empty() || !clip.iPlayMs || !std::isfinite(clip.fPlayRate) || clip.fPlayRate <= 0.f)
                Problem(row.status, "Missing clip or invalid timing.");
        }
        stageStart += stage.iDurationMs;
    }
    if (m_Rows.size() == firstRow) Problem(actor.status, "Empty model timeline (saved DRAFT may have no clips).");
    const double end = actor.effectiveOffsetMs + double(stageStart);
    m_DurationMs = (std::max)(m_DurationMs, static_cast<std::uint32_t>((std::min)(std::ceil(end), double(MAX_TIME_MS))));
}

void Client::CEffectCompositionModelPreview::Build_Rows()
{
    m_Actors.clear(); m_Rows.clear(); m_DurationMs = 0u; m_Status.clear();
    const auto findPattern = [&](const std::string& id) -> const KOUKU_SAYDON_COMPOSITION_PATTERN*
    {
        const auto found = std::find_if(m_Document.Patterns.begin(), m_Document.Patterns.end(),
            [&](const auto& row) { return row.strPatternId == id; });
        return found == m_Document.Patterns.end() ? nullptr : &*found;
    };
    if (m_SelectedId.empty()) return;
    if (m_IsBundle)
    {
        const auto bundle = std::find_if(m_Document.Bundles.begin(), m_Document.Bundles.end(),
            [&](const auto& row) { return row.strBundleId == m_SelectedId; });
        if (bundle == m_Document.Bundles.end())
        { m_Status = "Selected saved Bundle was removed."; return; }
        for (const auto& member : bundle->Members)
            Append_Actor(findPattern(member.strPatternId), member.strPatternId, member.strMemberId, member.iStartOffsetMs);
        m_Status = bundle->strLoadError;
    }
    else Append_Actor(findPattern(m_SelectedId), m_SelectedId, m_SelectedId, 0u);
    std::stable_sort(m_Rows.begin(), m_Rows.end(), [](const auto& a, const auto& b) { return a.startMs < b.startMs; });
    if (m_Status.empty()) m_Status = m_Rows.empty() ? "No saved animation clips in this selection." :
        "Read-only saved animation reference. Arena preview stays at the authored spawn; it does not reproduce Server movement.";
}

bool Client::CEffectCompositionModelPreview::Begin(const std::uint32_t clockMs, const bool paused)
{
    if (!m_Player || m_SelectedId.empty())
    { m_Status = "Select a saved Pattern/Bundle and enter the KoukuSaydon arena for model preview."; return false; }
    if (!m_Player->Begin_ModelReferencePreview(m_Document, m_SelectedId, m_IsBundle, clockMs, paused, m_Status)) return false;
    m_PreviewGeneration = m_Player->Preview_Generation();
    return true;
}

bool Client::CEffectCompositionModelPreview::Sample(const std::uint32_t clockMs, const bool paused)
{
    if (!Is_Active()) return false;
    m_Player->Sample_ModelReferencePreview(clockMs, paused);
    return true;
}

bool Client::CEffectCompositionModelPreview::Place_Root(const float4x4_t& root)
{
    if (!Is_Active() || m_Actors.size() != 1u) return false;
    for (const auto& row : root.m) for (float value : row) if (!std::isfinite(value)) return false;
    EFFECT_V2_TARGET target; EFFECT_V2_TARGET_VIEW view;
    if (!Resolve_Target(m_Actors.front().memberId, target, view)) return false;
    const auto actor = std::dynamic_pointer_cast<CNpc>(target.pOwner.lock());
    return actor && actor->Apply_NetworkState({root._41, root._42, root._43},
        XMConvertToDegrees(std::atan2(root._31, root._33)));
}

void Client::CEffectCompositionModelPreview::Stop()
{
    if (Is_Active()) m_Player->Stop_Preview();
    m_PreviewGeneration = 0u;
}

bool Client::CEffectCompositionModelPreview::Is_Active() const
{
    return m_Player && m_PreviewGeneration && m_Player->Preview_Generation() == m_PreviewGeneration &&
        m_Player->Preview_IsModelReference() && m_Player->Preview_Playing();
}

bool Client::CEffectCompositionModelPreview::Resolve_Target(const std::string& memberId,
    EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const
{
    return Is_Active() && m_Player->Resolve_ModelReferenceTarget(memberId, target, view);
}
