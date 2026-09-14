#include "EffectCompositionModelPreview.h"

#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "Npc.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "Level_KakulSaydonArena.h"
#include "ProjectDataRoot.h"

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
    // Keep only the stable Effect-to-pattern join used by model selection.
    // This projection does not admit or play these presentation resources.
    if (const auto* resources = root.Find("presentationResources"); resources && resources->Is_Array())
        for (const auto& value : resources->Get_Array())
        {
            std::string kind;
            KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
            if (Text(value, "kind", kind) && kind == "EFFECT" &&
                Text(value, "resourceId", resource.strResourceId) && !resource.strResourceId.empty() &&
                Text(value, "assetId", resource.strAssetId) && !resource.strAssetId.empty() &&
                Text(value, "resourceKind", resource.strResourceKind) &&
                (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT"))
                staged.PresentationResources.push_back(std::move(resource));
        }
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
        if (const auto* occurrences = value.Find("presentationOccurrences"); occurrences && occurrences->Is_Array())
            for (const auto& occurrenceValue : occurrences->Get_Array())
            {
                KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE occurrence;
                if (Text(occurrenceValue, "occurrenceId", occurrence.strOccurrenceId) && !occurrence.strOccurrenceId.empty() &&
                    Text(occurrenceValue, "resourceId", occurrence.strResourceId) &&
                    std::any_of(staged.PresentationResources.begin(), staged.PresentationResources.end(),
                        [&](const auto& resource) { return resource.strResourceId == occurrence.strResourceId; }))
                    pattern.PresentationOccurrences.push_back(std::move(occurrence));
            }
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

namespace
{
bool Read_SavedPropContext(const std::string& patternId,
    KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::shared_ptr<CWorldSequenceDocument>& sequences,
    std::string& status)
{
    const auto path = CKoukuSaydonCompositionDocument::Resolve_Path();
    std::error_code fileError;
    const auto bytes = std::filesystem::file_size(path, fileError);
    if (fileError || !bytes || bytes > 16u * 1024u * 1024u)
    { status = "Saved WORLD prop context is unavailable; previous model preserved."; return false; }
    std::ifstream input(path, std::ios::binary);
    std::string text(static_cast<std::size_t>(bytes), '\0');
    KOUKU_SAYDON_COMPOSITION_DOCUMENT staged;
    if (!input.read(text.data(), static_cast<std::streamsize>(text.size())) ||
        !CKoukuSaydonCompositionDocument::Parse_Text(text, staged, status))
    { status = "Saved WORLD prop context could not be read: " + status; return false; }
    const auto selected = std::find_if(staged.Patterns.begin(), staged.Patterns.end(),
        [&](const auto& row) { return row.strPatternId == patternId; });
    if (selected == staged.Patterns.end() || !selected->strLoadError.empty())
    {
        status = "Selected WORLD prop Pattern is missing or invalid: " + patternId;
        if (selected != staged.Patterns.end()) status += "; " + selected->strLoadError;
        return false;
    }
    if (selected->WorldOccurrences.empty())
    { status = "Selected Pattern has no saved WORLD props: " + patternId; return false; }
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level)
    { status = "Enter the KoukuSaydon arena before previewing saved WORLD props."; return false; }
    const auto& area = staged.strAreaId;
    if (!CWorldSequenceDocument::Is_ValidStableId(area) || area.find_first_of("/\\:") != std::string::npos)
    { status = "Saved WORLD prop Area ID is invalid."; return false; }
    const auto sequencesPath = CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") /
        area / (area + ".worldsequences.json"));
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    CWorldSequencePlayer::Collect_ValidationTargets(level->Get_CompositionWorldTargets(), placements, deploy);
    auto stagedSequences = std::make_shared<CWorldSequenceDocument>();
    if (!stagedSequences->Load(sequencesPath, area, placements, deploy, status))
    { status = "Saved WORLD prop definitions could not be admitted: " + status; return false; }
    document = std::move(staged);
    sequences = std::move(stagedSequences);
    return true;
}

std::vector<EFFECT_COMPOSITION_MODEL_PROP> Prop_Views(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE>& props, const CWorldSequenceDocument& sequences)
{
    std::vector<EFFECT_COMPOSITION_MODEL_PROP> result;
    for (const auto& box : props)
    {
        const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
            [&](const auto& value) { return value.strWorldId == box.strWorldId; });
        const auto* instance = sequences.Find_Instance(world->strSequenceInstanceId);
        const auto* object = sequences.Find_ObjectResource(instance->bindings.front().targetId);
        result.push_back({box.strOccurrenceId, box.strWorldId, instance->instanceId,
            world->strDisplayName, object->anchorBone, box.iStartMs, box.iDurationMs});
    }
    return result;
}
}

bool Client::CEffectCompositionModelPreview::Stage_ActorWorldProps(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const CWorldSequenceDocument& sequences, std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE>& props,
    std::string& status)
{
    std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE> staged;
    for (const auto& box : pattern.WorldOccurrences)
    {
        const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
            [&](const auto& value) { return value.strWorldId == box.strWorldId; });
        if (world == document.Worlds.end())
        { status = "Model-reference WORLD resource is missing: " + box.strWorldId; return false; }
        const auto* instance = sequences.Find_Instance(world->strSequenceInstanceId);
        if (!instance)
        { status = "Model-reference WORLD instance is missing: " + world->strSequenceInstanceId; return false; }
        const auto* motion = sequences.Find_Template(instance->templateId);
        if (!motion)
        { status = "Model-reference WORLD motion is missing: " + instance->templateId; return false; }
        if (!instance->enabled || instance->anchorKind != "BOSS" || instance->bindings.size() != 1u ||
            instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
            instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT || !instance->nextMotionId.empty() ||
            !motion->effectTracks.empty() || motion->objectMotion.EmissionCount() != 1u) continue;
        const auto* object = sequences.Find_ObjectResource(instance->bindings.front().targetId);
        if (!object)
        { status = "Model-reference WORLD object is missing: " + instance->bindings.front().targetId; return false; }
        if (object->anchorKind != "BOSS" || object->anchorBone.empty() ||
            object->anchorBossArchetypeId != CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
                pattern.strTargetBossPlacementId)) continue;
        staged.push_back(box);
    }
    if (staged.empty())
    { status = "Selected Pattern has no supported actor-bound WORLD props: " + pattern.strPatternId; return false; }
    props = std::move(staged);
    status.clear();
    return true;
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
    m_ReferenceWorldSequences.reset();
    m_Props.clear();
    m_Loaded = true;
    Build_Rows();
    m_Status = "Saved Composition rev " + std::to_string(m_Document.iRevision) + ": " +
        std::to_string(m_Document.Patterns.size()) + " patterns. " + (projected ? status : "Model rows do not require Product publish.");
    return true;
}

bool Client::CEffectCompositionModelPreview::Select_SourceEffect(const EFFECT_DOCUMENT_DESC& effect,
    const std::string& worldContextPatternId)
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
    std::shared_ptr<CWorldSequenceDocument> propSequences;
    std::vector<EFFECT_COMPOSITION_MODEL_PROP> propViews;
    if (!worldContextPatternId.empty())
    {
        KOUKU_SAYDON_COMPOSITION_DOCUMENT context;
        if (!Read_SavedPropContext(worldContextPatternId, context, propSequences, m_Status)) return false;
        const auto selected = std::find_if(context.Patterns.begin(), context.Patterns.end(),
            [&](const auto& row) { return row.strPatternId == worldContextPatternId; });
        if (selected->strActorProfileId != source.strActorProfileId ||
            selected->strTargetBossPlacementId != source.strTargetBossPlacementId)
        {
            m_Status = "Selected WORLD prop Pattern must match this Effect's source actor: " + worldContextPatternId;
            return false;
        }
        if (!Stage_ActorWorldProps(context, *selected, *propSequences, pattern.WorldOccurrences, m_Status)) return false;
        propViews = Prop_Views(context, pattern.WorldOccurrences, *propSequences);
        staged.strAreaId = context.strAreaId;
        staged.Worlds = std::move(context.Worlds);
    }
    const auto selected = pattern.strPatternId;
    staged.Patterns.push_back(std::move(pattern));
    Stop();
    m_Document = std::move(staged); m_SelectedId = selected; m_IsBundle = false; m_Loaded = true;
    m_ReferenceWorldSequences = std::move(propSequences);
    m_Props = std::move(propViews);
    Build_Rows();
    m_Status = "Loaded this Effect's source model and animation windows.";
    if (m_ReferenceWorldSequences) m_Status += " Saved WORLD prop context: " + worldContextPatternId;
    return true;
}

bool Client::CEffectCompositionModelPreview::Select_WorldEffectContext(const std::string& patternId)
{
    KOUKU_SAYDON_COMPOSITION_DOCUMENT context;
    std::shared_ptr<CWorldSequenceDocument> sequences;
    if (!Read_SavedPropContext(patternId, context, sequences, m_Status)) return false;
    const auto selected = std::find_if(context.Patterns.begin(), context.Patterns.end(),
        [&](const auto& row) { return row.strPatternId == patternId; });
    auto pattern = *selected;
    if (!Stage_ActorWorldProps(context, *selected, *sequences, pattern.WorldOccurrences, m_Status)) return false;
    auto props = Prop_Views(context, pattern.WorldOccurrences, *sequences);
    KOUKU_SAYDON_COMPOSITION_DOCUMENT staged;
    staged.iRevision = context.iRevision; staged.strAreaId = context.strAreaId;
    staged.Worlds = std::move(context.Worlds);
    pattern.LogicOccurrences.clear(); pattern.SummonOccurrences.clear();
    pattern.SceneProfileOccurrences.clear(); pattern.PresentationOccurrences.clear();
    staged.Patterns.push_back(std::move(pattern));
    Stop();
    m_Document = std::move(staged); m_SelectedId = patternId; m_IsBundle = false; m_Loaded = true;
    m_ReferenceWorldSequences = std::move(sequences); m_Props = std::move(props);
    Build_Rows();
    m_Status = "Loaded saved actor, animation and Object anchors: " + patternId;
    return true;
}

bool Client::CEffectCompositionModelPreview::Resolve_WorldPropPivot(
    const std::string& occurrenceId, float4x4_t& out) const
{
    if (!Is_Active() || m_Actors.size() != 1u || !m_ReferenceWorldSequences ||
        std::none_of(m_Props.begin(), m_Props.end(), [&](const auto& prop)
            { return prop.occurrenceId == occurrenceId; })) return false;
    return m_Player->Resolve_ModelReferenceWorldPivot(m_Actors.front().memberId, occurrenceId, out);
}

bool Client::CEffectCompositionModelPreview::Select_Pattern(const std::string& patternId)
{
    if (std::none_of(m_Document.Patterns.begin(), m_Document.Patterns.end(),
        [&](const auto& row) { return row.strPatternId == patternId; }))
    { m_Status = "Saved Pattern is missing; previous selection preserved."; return false; }
    Stop(); m_ReferenceWorldSequences.reset(); m_Props.clear(); m_SelectedId = patternId; m_IsBundle = false; Build_Rows(); return true;
}

bool Client::CEffectCompositionModelPreview::Select_Bundle(const std::string& bundleId)
{
    if (std::none_of(m_Document.Bundles.begin(), m_Document.Bundles.end(),
        [&](const auto& row) { return row.strBundleId == bundleId; }))
    { m_Status = "Saved Bundle is missing; previous selection preserved."; return false; }
    Stop(); m_ReferenceWorldSequences.reset(); m_Props.clear(); m_SelectedId = bundleId; m_IsBundle = true; Build_Rows(); return true;
}

void Client::CEffectCompositionModelPreview::Clear_Selection()
{
    Stop(); m_ReferenceWorldSequences.reset(); m_Props.clear(); m_SelectedId.clear(); m_IsBundle = false; Build_Rows();
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
    if (!m_Player->Begin_ModelReferencePreview(m_Document, m_SelectedId, m_IsBundle, clockMs, paused,
        m_Status, m_ReferenceWorldSequences.get())) return false;
    m_PreviewGeneration = m_Player->Preview_Generation();
    return true;
}

bool Client::CEffectCompositionModelPreview::Sample(const std::uint32_t clockMs, const bool paused)
{
    if (!Is_Active()) return false;
    m_Player->Sample_ModelReferencePreview(clockMs, paused);
    if (!Is_Active()) { m_Status = m_Player->Status(); return false; }
    return true;
}

bool Client::CEffectCompositionModelPreview::Place_Root(const float4x4_t& root)
{
    if (!Is_Active() || m_Actors.size() != 1u) return false;
    if (m_Player->Place_ModelReferenceRoot(root)) return true;
    m_Status = m_Player->Status();
    return false;
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
