#include "imgui.h"
#include "WorldObjectTool.h"

#ifdef _DEBUG
#include "CompositionTimeline.h"
#include "Animation.h"
#include "ActorCatalog.h"
#include "BinaryAsset/WModelDecoder.h"
#include "RuntimeAssetRoot.h"
#include "Level_KakulSaydonArena.h"
#include "ProjectDataRoot.h"
#include "WorldSequencePlayer.h"
#include "KoukuSaydonCompositionDocument.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>

namespace
{
constexpr const char* AREA_ID = "LV_LUT_MIDNIGHTC_ED";

bool ReadSource(const std::filesystem::path& path, std::string& bytes,
    std::string& status, const bool optional = false)
{
    std::error_code error;
    if (optional && !std::filesystem::exists(path, error) && !error)
    { bytes.clear(); return true; }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > 64u * 1024u * 1024u)
    { status = "Cannot read bounded authoring source: " + path.string(); return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Cannot open source: " + path.string(); return false; }
    bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    if (input.bad() || bytes.size() != size)
    { status = "Source changed or failed while reading: " + path.string(); return false; }
    return true;
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool EditText(const char* label, std::string& value, const size_t maxBytes = 128)
{
    char text[1024]{};
    std::snprintf(text, sizeof(text), "%s", value.c_str());
    if (!ImGui::InputText(label, text, (std::min)(sizeof(text), maxBytes + 1))) return false;
    value = text;
    return true;
}

bool EditUInt(const char* label, uint32_t& value, const int maximum, const int minimum = 0)
{
    int number = static_cast<int>((std::min)(value, static_cast<uint32_t>(INT_MAX)));
    if (!ImGui::DragInt(label, &number, 1.f, minimum, maximum, "%d", ImGuiSliderFlags_AlwaysClamp)) return false;
    value = static_cast<uint32_t>((std::clamp)(number, minimum, maximum));
    return true;
}

const char* MotionEndLabel(const Client::WORLD_SEQUENCE_MOTION_END motionEnd)
{
    switch (motionEnd)
    {
    case Client::WORLD_SEQUENCE_MOTION_END::STOP: return "Stop";
    case Client::WORLD_SEQUENCE_MOTION_END::HOLD: return "Hold Last Pose";
    case Client::WORLD_SEQUENCE_MOTION_END::LOOP: return "Loop";
    case Client::WORLD_SEQUENCE_MOTION_END::NEXT: return "Play Motion";
    default: return "Unknown";
    }
}

float3_t QuaternionEuler(const float4_t& q)
{
    float4x4_t matrix;
    XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(XMLoadFloat4(&q)));
    const float pitch = std::asin((std::clamp)(-matrix._32, -1.f, 1.f));
    const float cosine = std::cos(pitch);
    const float yaw = std::abs(cosine) > .00001f ? std::atan2(matrix._31, matrix._33) : std::atan2(-matrix._13, matrix._11);
    const float roll = std::abs(cosine) > .00001f ? std::atan2(matrix._12, matrix._22) : 0.f;
    return {XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
}

struct LinkedAuthoringWrite
{
    std::filesystem::path path;
    std::filesystem::path stagedPath;
    std::filesystem::path rollbackPath;
    std::string before;
    std::string after;
    bool committed = false;
};

bool WriteAuthoringStage(const std::filesystem::path& path, const std::string& bytes, std::string& status)
{
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output || !output.write(bytes.data(), static_cast<std::streamsize>(bytes.size())) || !output.flush())
    { status = "Cannot stage linked authoring file: " + path.string(); return false; }
    output.close();
    std::string reopened;
    return ReadSource(path, reopened, status) && reopened == bytes;
}

bool CommitAuthoringWrites(std::vector<LinkedAuthoringWrite>& writes, bool& preserveRecovery, std::string& status)
{
    preserveRecovery = false;
    for (const auto& write : writes)
    {
        std::string current;
        if (!ReadSource(write.path, current, status) || current != write.before)
        { status = "Linked authoring source changed during Save; existing edits preserved: " + write.path.string(); return false; }
    }
    for (auto& write : writes)
    {
        std::string current;
        const bool fresh = ReadSource(write.path, current, status) && current == write.before;
        if (!fresh || !MoveFileExW(write.stagedPath.c_str(), write.path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
            const std::string reason = fresh ? "Linked authoring replacement failed." : "Linked source changed before replacement.";
            bool restored = true;
            for (auto undo = writes.rbegin(); undo != writes.rend(); ++undo)
            {
                if (!undo->committed) continue;
                std::string actual;
                if (!ReadSource(undo->path, actual, status) || actual != undo->after ||
                    !MoveFileExW(undo->rollbackPath.c_str(), undo->path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) restored = false;
            }
            preserveRecovery = !restored;
            status = reason + (restored ? " Previous sources restored; draft preserved." :
                " Concurrent source edits were preserved; rollback files remain for recovery. Reload before saving again.");
            return false;
        }
        write.committed = true;
    }
    return true;
}

bool SynchronizeEmissionReferences(Client::KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const Client::CWorldSequenceDocument& previous, const Client::CWorldSequenceDocument& edited,
    const std::map<std::string, std::vector<uint32_t>>& provenance,
    const std::set<std::string>& editedMotions, bool& hasReferences, std::string& status)
{
    using namespace Client;
    const auto fail = [&](const std::string& reason) { status = "Object Save preserved all sources: " + reason; return false; };
    // Result/Contact motions and NEXT chains are consumers even when no WORLD
    // box names that Motion directly. Their existing runtime owns one object.
    const auto registerMotion = [&](const std::string& firstId, bool singleObject) {
        std::vector<std::string> pending{firstId};
        std::set<std::string> seen;
        while (!pending.empty())
        {
            const auto id = pending.back(); pending.pop_back();
            if (id.empty() || !seen.insert(id).second) continue;
            const auto* before = previous.Find_Instance(id);
            const auto* after = edited.Find_Instance(id);
            if (before && editedMotions.contains(before->templateId))
            {
                hasReferences = true;
                const auto* motion = after ? edited.Find_Template(after->templateId) : nullptr;
                if (!after || !after->enabled || !motion) return fail("a Logic/World Motion was removed or disabled: " + id);
                if (singleObject && motion->objectMotion.EmissionCount() != 1u)
                    return fail("Logic motion changes require Count 1: " + id + ". Keep each target as its own WORLD box.");
            }
            for (const auto* instance : {before, after})
                if (instance && instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) pending.push_back(instance->nextMotionId);
        }
        return true;
    };
    const auto registerContactTarget = [&](const std::string& occurrenceId) {
        for (const auto& pattern : document.Patterns)
            for (const auto& box : pattern.WorldOccurrences)
                if (box.strOccurrenceId == occurrenceId)
                    for (const auto& world : document.Worlds)
                        if (world.strWorldId == box.strWorldId && !registerMotion(world.strSequenceInstanceId, true)) return false;
        return true;
    };
    for (const auto& logic : document.Logics)
    {
        for (const auto& target : logic.TargetWorldOccurrenceIds) if (!registerContactTarget(target)) return false;
        if (!registerMotion(logic.strWorldSequenceInstanceId, false) ||
            !registerMotion(logic.strTargetWorldInstanceId, true) || !registerMotion(logic.strMotionInstanceId, true)) return false;
        for (const auto& contact : logic.ContactMotions)
            if (!registerMotion(contact.strMotionInstanceId, true) || !registerContactTarget(contact.strTargetWorldOccurrenceId)) return false;
    }
    for (const auto& world : document.Worlds)
        if (!registerMotion(world.strSequenceInstanceId, false)) return false;
    for (auto& pattern : document.Patterns)
    {
        if (!pattern.strLoadError.empty())
            for (const auto& world : document.Worlds)
                if (const auto* instance = previous.Find_Instance(world.strSequenceInstanceId);
                    instance && editedMotions.contains(instance->templateId) && pattern.strPreservedJson.find(world.strWorldId) != std::string::npos)
                    return fail("repair referenced Pattern " + pattern.strPatternId + " before editing its Motion.");
        for (const auto& worldBox : pattern.WorldOccurrences)
        {
            const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& row) { return row.strWorldId == worldBox.strWorldId; });
            if (world == document.Worlds.end()) continue;
            const auto* beforeInstance = previous.Find_Instance(world->strSequenceInstanceId);
            const auto* afterInstance = edited.Find_Instance(world->strSequenceInstanceId);
            if (!beforeInstance || !editedMotions.contains(beforeInstance->templateId)) continue;
            hasReferences = true;
            if (!pattern.strLoadError.empty()) return fail("repair referenced Pattern " + pattern.strPatternId + " before editing its Motion.");
            const auto* before = previous.Find_Template(beforeInstance->templateId);
            const auto* after = afterInstance ? edited.Find_Template(afterInstance->templateId) : nullptr;
            if (!before || !after) return fail("a referenced Motion was removed: " + world->strSequenceInstanceId);
            const auto& oldMotion = before->objectMotion;
            const auto& newMotion = after->objectMotion;
            std::vector<uint32_t> origins;
            if (const auto found = provenance.find(after->sequenceId); found != provenance.end()) origins = found->second;
            else for (uint32_t row = 0; row < newMotion.EmissionCount(); ++row) origins.push_back(row);
            if (origins.size() != newMotion.EmissionCount() || std::any_of(origins.begin(), origins.end(),
                [&](uint32_t row) { return row >= oldMotion.EmissionCount(); }))
                return fail("emission provenance is invalid for " + after->displayName + ". Reload the Object source.");
            const auto belongs = [&](const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& box) {
                if (box.strAnchorKind != "WORLD" || box.strWorldId != worldBox.strWorldId) return false;
                if (!box.strWorldOccurrenceId.empty()) return box.strWorldOccurrenceId == worldBox.strOccurrenceId;
                return std::count_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
                    [&](const auto& other) { return other.strWorldId == worldBox.strWorldId; }) == 1;
            };
            const auto isCollider = [&](const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& box) {
                const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                    [&](const auto& row) { return row.strResourceId == box.strResourceId; });
                return resource != document.PresentationResources.end() && resource->eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
            };
            if (std::count_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
                [&](const auto& other) { return other.strWorldId == worldBox.strWorldId; }) != 1 &&
                std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(), [&](const auto& box) {
                    return box.strAnchorKind == "WORLD" && box.strWorldId == worldBox.strWorldId && box.strWorldOccurrenceId.empty() && isCollider(box); }))
                return fail("Collider has an ambiguous WORLD occurrence. Select its exact Object box in Composition first.");
            std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE> nextColliders;
            std::map<std::pair<std::string, uint32_t>, std::string> linkedLogics;
            std::set<std::string> removedLogicCandidates;
            const auto originalLogics = pattern.LogicOccurrences;
            const auto shiftStart = [&](uint32_t start, int64_t shift, uint32_t& out) {
                const int64_t result = int64_t(start) + shift;
                if (result < 0 || result > CWorldSequenceDocument::MAX_DURATION_MS) return false;
                out = static_cast<uint32_t>(result); return true;
            };
            for (const auto& source : pattern.PresentationOccurrences)
            {
                if (!belongs(source) || !isCollider(source)) { nextColliders.push_back(source); continue; }
                if (source.iWorldEmissionIndex >= oldMotion.EmissionCount())
                    return fail("Collider names an absent original emission: " + source.strOccurrenceId);
                if (newMotion.emissions.empty() && newMotion.EmissionCount() != 1u)
                    return fail("WORLD Collider groups need authored emission rows. Use Group Layout / Resize Group before saving.");
                std::vector<uint32_t> targets;
                for (uint32_t row = 0; row < origins.size(); ++row)
                    if (origins[row] == source.iWorldEmissionIndex) targets.push_back(row);
                if (targets.empty())
                {
                    if (!source.strLogicOccurrenceId.empty()) removedLogicCandidates.insert(source.strLogicOccurrenceId);
                    continue;
                }
                for (size_t copyIndex = 0; copyIndex < targets.size(); ++copyIndex)
                {
                    const auto target = targets[copyIndex];
                    const double rate = worldBox.fPlaybackSpeed;
                    if (!std::isfinite(rate) || rate <= 0. || beforeInstance->playbackSpeed <= 0.f || afterInstance->playbackSpeed <= 0.f)
                        return fail("WORLD playback speed is invalid.");
                    const double delta = (double(newMotion.EmissionDelayMs(target)) / afterInstance->playbackSpeed -
                        double(oldMotion.EmissionDelayMs(source.iWorldEmissionIndex)) / beforeInstance->playbackSpeed) / rate;
                    if (!std::isfinite(delta) || std::abs(delta) > CWorldSequenceDocument::MAX_DURATION_MS)
                        return fail("emission time change is outside the timeline limit.");
                    const int64_t shift = std::llround(delta);
                    auto collider = source;
                    collider.iWorldEmissionIndex = target;
                    if (!shiftStart(source.iStartMs, shift, collider.iStartMs))
                        return fail("emission delay moves Collider before zero or beyond the timeline: " + source.strOccurrenceId);
                    if (collider.iStartMs < worldBox.iStartMs ||
                        uint64_t(collider.iStartMs) + collider.iDurationMs > uint64_t(worldBox.iStartMs) + worldBox.iDurationMs)
                        return fail("emission delay moves Collider outside its WORLD box lifetime: " + source.strOccurrenceId +
                            ". Extend that WORLD box in Composition first.");
                    if (copyIndex)
                    {
                        do {
                            if (pattern.iNextPresentationOccurrenceOrdinal == UINT32_MAX) return fail("Collider identity range is exhausted.");
                            collider.strOccurrenceId = pattern.strPatternId + ".presentation." + std::to_string(pattern.iNextPresentationOccurrenceOrdinal++);
                        } while (std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                            [&](const auto& row) { return row.strOccurrenceId == collider.strOccurrenceId; }) ||
                            std::any_of(nextColliders.begin(), nextColliders.end(), [&](const auto& row) { return row.strOccurrenceId == collider.strOccurrenceId; }));
                    }
                    if (!source.strLogicOccurrenceId.empty())
                    {
                        const auto key = std::make_pair(source.strLogicOccurrenceId, target);
                        if (const auto linked = linkedLogics.find(key); linked != linkedLogics.end()) collider.strLogicOccurrenceId = linked->second;
                        else
                        {
                            const auto logic = std::find_if(originalLogics.begin(), originalLogics.end(),
                                [&](const auto& row) { return row.strOccurrenceId == source.strLogicOccurrenceId; });
                            if (logic == originalLogics.end()) return fail("Collider Logic is missing: " + source.strLogicOccurrenceId);
                            if (copyIndex || shift)
                            {
                                const bool mixed = std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                                    [&](const auto& row) { return row.strLogicOccurrenceId == source.strLogicOccurrenceId &&
                                        (!belongs(row) || !isCollider(row) || row.iWorldEmissionIndex != source.iWorldEmissionIndex); });
                                const bool held = !logic->strHoldLogicOccurrenceId.empty() ||
                                    std::any_of(originalLogics.begin(), originalLogics.end(), [&](const auto& row) { return row.strHoldLogicOccurrenceId == logic->strOccurrenceId; }) ||
                                    std::any_of(document.Logics.begin(), document.Logics.end(), [&](const auto& row) { return row.strTargetLogicOccurrenceId == logic->strOccurrenceId; });
                                if (mixed || held) return fail("Motion row shares a Logic dependency with another row or hold/result target: " + logic->strOccurrenceId +
                                    ". Separate that Logic in Composition before changing this row's count or delay.");
                            }
                            auto nextLogic = *logic;
                            if (!shiftStart(logic->iStartMs, shift, nextLogic.iStartMs)) return fail("emission delay moves Logic outside the timeline.");
                            if (copyIndex)
                            {
                                do {
                                    if (pattern.iNextLogicOccurrenceOrdinal == UINT32_MAX) return fail("Logic identity range is exhausted.");
                                    nextLogic.strOccurrenceId = pattern.strPatternId + ".logic." + std::to_string(pattern.iNextLogicOccurrenceOrdinal++);
                                } while (std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
                                    [&](const auto& row) { return row.strOccurrenceId == nextLogic.strOccurrenceId; }));
                                pattern.LogicOccurrences.push_back(nextLogic);
                            }
                            else
                                for (auto& row : pattern.LogicOccurrences) if (row.strOccurrenceId == nextLogic.strOccurrenceId) { row = nextLogic; break; }
                            collider.strLogicOccurrenceId = nextLogic.strOccurrenceId;
                            linkedLogics.emplace(key, nextLogic.strOccurrenceId);
                        }
                    }
                    nextColliders.push_back(std::move(collider));
                }
            }
            pattern.PresentationOccurrences = std::move(nextColliders);
            for (const auto& id : removedLogicCandidates)
            {
                if (std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                    [&](const auto& row) { return row.strLogicOccurrenceId == id; })) continue;
                if (std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto& row) { return row.strHoldLogicOccurrenceId == id; }) ||
                    std::any_of(document.Logics.begin(), document.Logics.end(), [&](const auto& row) { return row.strTargetLogicOccurrenceId == id; }))
                    return fail("deleted emission owns a Logic used by a hold/result target: " + id + ". Reconnect it in Composition first.");
                std::erase_if(pattern.LogicOccurrences, [&](const auto& row) { return row.strOccurrenceId == id; });
            }
        }
    }
    return true;
}

}

using namespace Client;

CWorldObjectTool::~CWorldObjectTool()
{
    Stop_Preview();
    if (m_PublishProcess) CloseHandle(m_PublishProcess);
}

void CWorldObjectTool::Open()
{
    m_Open = true;
    m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
    if (!m_Ready) Load_Source();
}

bool CWorldObjectTool::Open_ObjectMotion(const std::string& objectId,
    const std::string& instanceId, std::string& status)
{
    if (!m_Ready && !Load_Source()) { status = m_Status; return false; }
    const auto* resource = m_Document.Find_ObjectResource(objectId);
    if (!resource)
    { status = m_Status = "The selected Object is absent from the current Object Tool draft. Existing edits are preserved."; return false; }
    if (!instanceId.empty())
    {
        const auto states = StateIds(*resource);
        if (std::find(states.begin(), states.end(), instanceId) == states.end())
        { status = m_Status = "The selected Motion does not belong to this Object in the current draft. Existing edits are preserved."; return false; }
    }
    Open();
    Select_Object(objectId);
    if (!instanceId.empty()) Select_State(instanceId);
    status = m_Status = instanceId.empty() ? "Opened Object settings. Save keeps edits." :
        "Opened the selected Motion. Emissions, movement and rotation are shared by every box using this Motion.";
    return true;
}

bool CWorldObjectTool::Consume_InteractionRequest()
{
    const bool requested = m_InteractionRequested;
    m_InteractionRequested = false;
    return requested;
}

void CWorldObjectTool::Deactivate()
{
    Stop_Preview();
}

bool CWorldObjectTool::Load_Source()
{
    const auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level)
    { m_Status = "Enter KoukuSaydon, then Reload Source to edit and preview world objects."; return false; }
    const auto directory = CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") / AREA_ID);
    const auto sourcePath = directory / (std::string(AREA_ID) + ".worldsequences.json");
    const auto placementPath = directory / (std::string(AREA_ID) + ".mapplacements");
    const auto deployPath = directory / (std::string(AREA_ID) + ".deployplacements");
    std::string sourceBefore, mapBefore, deployBefore;
    if (!ReadSource(sourcePath, sourceBefore, m_Status) ||
        !ReadSource(placementPath, mapBefore, m_Status) ||
        !ReadSource(deployPath, deployBefore, m_Status, true)) return false;
    WORLD_SEQUENCE_PLACEMENT_MAP map;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    level->Get_WorldObjectValidationTargets(map, deploy);
    CWorldSequenceDocument staged;
    if (!staged.Load(sourcePath, AREA_ID, map, deploy, m_Status)) return false;
    std::string sourceAfter, mapAfter, deployAfter;
    if (!ReadSource(sourcePath, sourceAfter, m_Status) ||
        !ReadSource(placementPath, mapAfter, m_Status) ||
        !ReadSource(deployPath, deployAfter, m_Status, true)) return false;
    if (sourceBefore != sourceAfter || mapBefore != mapAfter || deployBefore != deployAfter)
    { m_Status = "Linked authoring source changed during Reload; existing draft preserved."; return false; }
    Stop_Preview();
    m_Document = std::move(staged);
    m_SavedDocument = m_Document;
    m_MapTargets = std::move(map);
    m_DeployTargets = std::move(deploy);
    m_SourcePath = sourcePath; m_PlacementPath = placementPath; m_DeployPath = deployPath;
    m_SourceBytes = std::move(sourceAfter); m_PlacementBytes = std::move(mapAfter); m_DeployBytes = std::move(deployAfter);
    m_Ready = true; m_Dirty = false; ++m_SavedGeneration;
    m_EmissionOrigins.clear(); m_EditedMotionIds.clear();
    m_PristinePatternId.clear();
    m_AnimationObjectId.clear();
    m_AnimationCandidateObjectId.clear();
    m_AnimationCandidateModelAssetId.clear();
    if (Preview_Group()) m_SelectedObject = m_SelectedGroup;
    if (!m_Document.Find_ObjectResource(m_SelectedObject))
        m_SelectedObject = m_Document.Get_ObjectResources().empty() ? "" : m_Document.Get_ObjectResources().front().objectId;
    Select_Object(m_SelectedObject);
    m_Status = "Source loaded. Save stores Object edits and applies them for the next play.";
    return true;
}

bool CWorldObjectTool::Matches_SourceBaseline()
{
    std::string source, map, deploy;
    if (!ReadSource(m_SourcePath, source, m_Status) ||
        !ReadSource(m_PlacementPath, map, m_Status) ||
        !ReadSource(m_DeployPath, deploy, m_Status, true)) return false;
    if (source != m_SourceBytes || map != m_PlacementBytes || deploy != m_DeployBytes)
    { m_Status = "Save conflict: linked source changed on disk. Draft preserved; Reload Source before saving."; return false; }
    return true;
}

bool CWorldObjectTool::Save_Source()
{
    if (!m_Ready || m_PublishProcess || !Matches_SourceBaseline()) return false;
    auto stagedPath = m_SourcePath;
    const auto suffix = L".world-object-" + std::to_wstring(GetCurrentProcessId());
    stagedPath += suffix + L".stage";
    if (!m_Document.Save(stagedPath, m_MapTargets, m_DeployTargets, m_Status)) return false;
    CWorldSequenceDocument verified;
    std::string stagedBytes;
    std::vector<LinkedAuthoringWrite> writes;
    const auto cleanup = [&]() {
        std::error_code ignored;
        std::filesystem::remove(stagedPath, ignored);
        for (const auto& write : writes)
        {
            std::filesystem::remove(write.stagedPath, ignored);
            std::filesystem::remove(write.rollbackPath, ignored);
        }
    };
    if (!verified.Load(stagedPath, AREA_ID, m_MapTargets, m_DeployTargets, m_Status) ||
        !m_Document.Is_Equivalent(verified) || !ReadSource(stagedPath, stagedBytes, m_Status) || !Matches_SourceBaseline())
    { cleanup(); return false; }
    bool linked = false, publishPatterns = false;
    const std::array paths{CKoukuSaydonCompositionDocument::Resolve_Path(), CKoukuSaydonCompositionDocument::Resolve_SequencePath()};
    for (size_t index = 0; index < paths.size(); ++index)
    {
        std::error_code error;
        if (!std::filesystem::exists(paths[index], error) && !error) continue;
        LinkedAuthoringWrite write;
        write.path = paths[index];
        if (!ReadSource(write.path, write.before, m_Status)) { cleanup(); return false; }
        CKoukuSaydonCompositionDocument owner(write.path);
        if (!owner.Reload(m_Status)) { cleanup(); return false; }
        auto candidate = owner.Get_LastGood();
        bool references = false;
        if (!SynchronizeEmissionReferences(candidate, m_SavedDocument, verified,
            m_EmissionOrigins, m_EditedMotionIds, references, m_Status)) { cleanup(); return false; }
        linked = linked || references;
        publishPatterns = publishPatterns || (index == 0 && references);
        if (candidate == owner.Get_LastGood()) continue;
        if (candidate.iRevision == UINT32_MAX)
        { m_Status = "Linked Composition revision is exhausted; all sources preserved."; cleanup(); return false; }
        ++candidate.iRevision;
        if (!CKoukuSaydonCompositionDocument::Validate(candidate, owner.Get_References(), m_Status))
        { cleanup(); return false; }
        write.after = CKoukuSaydonCompositionDocument::Serialize(candidate);
        KOUKU_SAYDON_COMPOSITION_DOCUMENT reopened;
        if (!CKoukuSaydonCompositionDocument::Parse_Text(write.after, reopened, m_Status) ||
            !CKoukuSaydonCompositionDocument::Validate(reopened, owner.Get_References(), m_Status) ||
            CKoukuSaydonCompositionDocument::Serialize(reopened) != write.after)
        { cleanup(); return false; }
        write.stagedPath = write.path; write.stagedPath += suffix + L".stage";
        write.rollbackPath = write.path; write.rollbackPath += suffix + L".rollback";
        writes.push_back(std::move(write));
    }
    if (linked && (!m_CanSaveLinked || !m_CanSaveLinked(m_Status)))
    { if (m_Status.empty()) m_Status = "Linked Composition editor is unavailable. Sources preserved."; cleanup(); return false; }
    LinkedAuthoringWrite worldWrite;
    worldWrite.path = m_SourcePath; worldWrite.before = m_SourceBytes; worldWrite.after = stagedBytes;
    worldWrite.stagedPath = stagedPath; worldWrite.rollbackPath = m_SourcePath; worldWrite.rollbackPath += suffix + L".rollback";
    writes.push_back(std::move(worldWrite));
    for (const auto& write : writes)
        if (!WriteAuthoringStage(write.stagedPath, write.after, m_Status) ||
            !WriteAuthoringStage(write.rollbackPath, write.before, m_Status)) { cleanup(); return false; }
    bool preserveRecovery = false;
    if (!Matches_SourceBaseline() || !CommitAuthoringWrites(writes, preserveRecovery, m_Status))
    { if (!preserveRecovery) cleanup(); return false; }
    cleanup();
    m_SourceBytes = std::move(stagedBytes); m_Document = std::move(verified);
    m_SavedDocument = m_Document; m_Dirty = false; ++m_SavedGeneration;
    m_EmissionOrigins.clear(); m_EditedMotionIds.clear();
    m_PristinePatternId.clear();
    m_LinkedSavePending = m_LinkedSavePending || linked;
    m_PublishLinkedPatterns = m_PublishLinkedPatterns || publishPatterns;
    m_Status = "Saved Object and linked Collider/Logic rows; applying World Object runtime data.";
    Start_Publish();
    return true;
}

void CWorldObjectTool::Start_Publish()
{
    if (!m_Ready || m_Dirty || m_PublishProcess) return;
    if (!Matches_SourceBaseline())
    { m_Status = "Saved; apply stopped because linked source changed. Reload Source before retrying."; return; }
    const auto root = CProjectDataRoot::Get().parent_path();
    const auto script = root / L"Tools/MapPipeline/Publish-MapAuthoring.ps1";
    if (!std::filesystem::is_regular_file(script)) { m_Status = "Saved; apply failed: publisher is missing. Save to retry."; return; }
    wchar_t temporary[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temporary)) { m_Status = "Saved; apply failed: log folder is unavailable. Save to retry."; return; }
    m_PublishLog = std::filesystem::path(temporary) / (L"LostArk-WorldObject-" + std::to_wstring(GetCurrentProcessId()) + L".log");
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    const HANDLE log = CreateFileW(m_PublishLog.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == log) { m_Status = "Saved; apply failed: cannot create log. Save to retry."; return; }
    const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == input) { CloseHandle(log); m_Status = "Saved; apply failed: cannot prepare input. Save to retry."; return; }
    std::wstring command = L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" +
        script.wstring() + L"\" -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish";
    std::vector<wchar_t> arguments(command.begin(), command.end()); arguments.push_back(0);
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = log; startup.hStdError = log; startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const bool started = !!CreateProcessW(nullptr, arguments.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process);
    CloseHandle(log); CloseHandle(input);
    if (!started) { m_Status = "Saved; apply failed: cannot start publisher. Save to retry."; return; }
    CloseHandle(process.hThread); m_PublishProcess = process.hProcess;
    m_Status = "Saved; applying World Object runtime data. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Poll_Publish()
{
    if (!m_PublishProcess || WaitForSingleObject(m_PublishProcess, 0) == WAIT_TIMEOUT) return;
    DWORD code = 1; GetExitCodeProcess(m_PublishProcess, &code);
    CloseHandle(m_PublishProcess); m_PublishProcess = nullptr;
    if (code != 0)
    { m_Status = "Saved; apply failed (" + std::to_string(code) + "). Previous runtime preserved. Save to retry. Log: " + m_PublishLog.string(); return; }
    ++m_SavedGeneration;
    std::string runtimeStatus;
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        if (!level->Reload_WorldObjectRuntime(runtimeStatus))
        { m_Status = "Saved and runtime files applied; next-play reload pending: " + runtimeStatus; return; }
    }
    if (m_LinkedSavePending)
    {
        std::string status;
        if (!m_ApplyLinkedSave || !m_ApplyLinkedSave(m_PublishLinkedPatterns, status))
        { m_Status = "Object sources and Map applied; linked Composition apply is pending: " + status + " " + runtimeStatus; return; }
        m_LinkedSavePending = false; m_PublishLinkedPatterns = false;
        m_Status = "Object and linked sources saved. " + status + " " + runtimeStatus;
        return;
    }
    m_Status = "Saved and applied for the next play. " + runtimeStatus + " Log: " + m_PublishLog.string();
}

std::vector<uint32_t>& CWorldObjectTool::Emission_Origins(const WORLD_SEQUENCE_TEMPLATE& sequence)
{
    const auto [entry, inserted] = m_EmissionOrigins.try_emplace(sequence.sequenceId);
    if (inserted)
    {
        entry->second.reserve(sequence.objectMotion.EmissionCount());
        for (uint32_t index = 0; index < sequence.objectMotion.EmissionCount(); ++index)
            entry->second.push_back(index);
    }
    return entry->second;
}

void CWorldObjectTool::Mark_Dirty()
{
    m_PristinePatternId.clear();
    if (const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject))
        for (const auto& id : StateIds(*resource))
            if (const auto* instance = m_Document.Find_Instance(id)) m_EditedMotionIds.insert(instance->templateId);
    m_Document.Touch(); m_Dirty = true; m_PreviewDirty = m_PreviewActive;
}

void CWorldObjectTool::Stop_Preview()
{
    if (m_PreviewActive && m_PreviewLevel && m_PreviewLevel == CLevel_KakulSaydonArena::Get_Active())
        m_PreviewLevel->Debug_StopWorldObjectPreview();
    m_PreviewLevel = nullptr; m_PreviewActive = false; m_Playing = false; m_PreviewDirty = false;
}

const WORLD_SEQUENCE_INSTANCE* CWorldObjectTool::Preview_Instance() const
{
    if (Preview_Group()) return nullptr;
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance.empty() && resource ?
        resource->defaultMotionInstanceId : m_SelectedInstance);
    return instance && instance->enabled ? instance : nullptr;
}

const WORLD_SEQUENCE_OBJECT_RESOURCE* CWorldObjectTool::Preview_Group() const
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedGroup);
    return resource && !resource->motionInstanceIds.empty() ? resource : nullptr;
}

bool CWorldObjectTool::Begin_Preview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) { m_Status = "World object preview requires the active KoukuSaydon arena."; return false; }
    if (const auto* group = Preview_Group())
    {
        if (SpanMs() <= 0.f)
        {
            Stop_Preview(); m_ClockMs = 0.f;
            m_Status = m_PreviewStatus = "All group motions are disabled.";
            return false;
        }
        if (!level->Debug_BeginWorldObjectPreview(m_Document, group->objectId, m_Status, m_PreviewAtCharacter)) return false;
        m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
        return true;
    }
    const auto* instance = Preview_Instance();
    if (!instance) { m_Status = "Choose an enabled Default Motion or select a connected Motion."; return false; }
    if (!level->Debug_BeginWorldObjectPreview(m_Document, instance->instanceId, m_Status, m_PreviewAtCharacter)) return false;
    m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
    return true;
}

f32_t CWorldObjectTool::SpanMs() const
{
    if (const auto* group = Preview_Group())
    {
        float span = 0.f;
        for (const auto& id : group->motionInstanceIds)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance && instance->enabled ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (sequence) span = (std::max)(span, static_cast<float>(instance->startDelayMs) +
                static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed));
        }
        return span;
    }
    const auto* instance = Preview_Instance();
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    return !sequence ? 0.f : static_cast<float>(instance->startDelayMs) +
        static_cast<float>(sequence->PresentationSpanMs()) / (std::max)(.05f, instance->playbackSpeed);
}

f32_t CWorldObjectTool::PreviewSpanMs() const
{
    const auto* instance = Preview_Instance();
    if (instance && (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP ||
        instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        return (std::max)(SpanMs(), static_cast<float>(CWorldSequenceDocument::MAX_DURATION_MS));
    return SpanMs();
}

void CWorldObjectTool::Seek(const f32_t clockMs)
{
    m_ClockMs = (std::clamp)(clockMs, 0.f, PreviewSpanMs());
    if ((!m_PreviewActive || m_PreviewDirty) && !Begin_Preview()) { m_Playing = false; return; }
    if (!m_PreviewLevel->Debug_SampleWorldObjectPreview(m_ClockMs, m_PreviewStatus))
    { m_Status = m_PreviewStatus; Stop_Preview(); }
}

void CWorldObjectTool::Update(const f32_t seconds, const bool_t active)
{
    Poll_Publish();
    if (m_PhysicalScanRunning && m_PhysicalScan.Advance())
    {
        if (m_PhysicalScan.Commit(m_PhysicalAssets, m_PhysicalStatus)) Rebuild_PhysicalTree();
        m_PhysicalScanRunning = false;
    }
    if (!active || !m_Open || (m_PreviewLevel && m_PreviewLevel != CLevel_KakulSaydonArena::Get_Active()))
    { Stop_Preview(); return; }
    if (m_PreviewActive && m_PreviewDirty) Seek(m_ClockMs);
    if (!m_Playing || !m_PreviewActive) return;
    const float span = PreviewSpanMs();
    m_ClockMs += (std::max)(0.f, seconds) * 1000.f;
    if (span <= 0.f) { Stop_Preview(); return; }
    if (m_ClockMs >= span) { m_ClockMs = span; m_Playing = false; }
    Seek(m_ClockMs);
}

std::vector<std::string> CWorldObjectTool::StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const
{
    if (!resource.motionInstanceIds.empty()) return resource.motionInstanceIds;
    if (!resource.sequenceInstanceId.empty()) return {resource.sequenceInstanceId};
    std::vector<std::string> ids;
    for (const auto& instance : m_Document.Get_Instances())
        if (std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
            return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == resource.objectId;
        })) ids.push_back(instance.instanceId);
    return ids;
}

void CWorldObjectTool::Select_Object(const std::string& id)
{
    Stop_Preview(); m_SelectedObject = id; m_SelectedInstance.clear(); m_ClockMs = 0.f;
    m_SelectedGroup.clear();
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (resource && !resource->motionInstanceIds.empty())
    {
        m_SelectedGroup = resource->objectId;
        m_PreviewAtCharacter = false;
        Select_State(resource->motionInstanceIds.front());
    }
    m_SelectedTrack = 0; m_SelectedKey = 0;
}

void CWorldObjectTool::Select_State(const std::string& id)
{
    if (const auto* group = Preview_Group())
    {
        const auto* instance = m_Document.Find_Instance(id);
        if (std::find(group->motionInstanceIds.begin(), group->motionInstanceIds.end(), id) == group->motionInstanceIds.end() ||
            !instance || instance->bindings.size() != 1u ||
            !m_Document.Find_ObjectResource(instance->bindings.front().targetId))
        { m_Status = "The selected motion does not belong to this combined motion."; return; }
        // Editing another row must not stop, rewind or solo the combined preview.
        m_SelectedObject = instance->bindings.front().targetId;
        m_SelectedInstance = id;
        m_SelectedTrack = 0; m_SelectedKey = 0; m_SelectedEffectRow = 0;
        return;
    }
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto motions = resource ? StateIds(*resource) : std::vector<std::string>{};
    if (std::find(motions.begin(), motions.end(), id) == motions.end())
    { m_Status = "The selected motion does not belong to this object."; return; }
    Stop_Preview(); m_SelectedInstance = id; m_SelectedTrack = 0; m_SelectedKey = 0; m_ClockMs = 0.f;
    if (const auto* instance = m_Document.Find_Instance(id))
        if (const auto* sequence = m_Document.Find_Template(instance->templateId))
            m_GroupCount = static_cast<int>(sequence->objectMotion.EmissionCount());
}

bool CWorldObjectTool::Create_Object()
{
    if (!m_NewObjectName[0]) { m_Status = "Enter an object name."; return false; }
    WORLD_SEQUENCE_OBJECT_RESOURCE resource;
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        resource.objectId = "world.object.resource." + std::to_string(index);
        if (!m_Document.Find_ObjectResource(resource.objectId)) break;
    }
    resource.displayName = m_NewObjectName.data();
    resource.anchorKind = m_NewObjectAnchor == 2 ? "BOSS" : m_NewObjectAnchor == 1 ? "PLAYER" : "WORLD";
    if (m_Document.Get_ObjectResources().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World object resource capacity reached."; return false; }
    m_Document.Get_ObjectResources().push_back(resource);
    Mark_Dirty(); Select_Object(resource.objectId);
    m_Status = "Object created. Assign its shared model, then Create Motion in Object Detail. Assign Model before Save.";
    m_NewObjectName[0] = 0;
    m_NewStateName[0] = 0;
    return true;
}

void CWorldObjectTool::Create_State()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty() || !m_NewStateName[0]) return;
    WORLD_SEQUENCE_TEMPLATE sequence;
    WORLD_SEQUENCE_INSTANCE instance;
    if (!Build_State(*resource, m_NewStateName.data(), sequence, instance)) return;
    const bool firstMotion = StateIds(*resource).empty();
    m_Document.Get_Templates().push_back(std::move(sequence));
    m_Document.Get_Instances().push_back(instance);
    if (firstMotion) resource->defaultMotionInstanceId = instance.instanceId;
    Mark_Dirty(); Select_State(instance.instanceId);
    m_PristinePatternId = instance.instanceId;
    m_NewStateName[0] = 0;
    m_Status = "Motion created. Append Clip or edit Transform/Physics, then Save.";
}

bool CWorldObjectTool::Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const std::string& stateName, WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance)
{
    if (m_Document.Get_Templates().size() >= CWorldSequenceDocument::MAX_TEMPLATE_COUNT ||
        m_Document.Get_Instances().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World sequence document capacity reached."; return false; }
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        sequence.sequenceId = resource.objectId + ".state." + std::to_string(index);
        instance.instanceId = sequence.sequenceId + ".instance";
        if (!m_Document.Find_Template(sequence.sequenceId) && !m_Document.Find_Instance(instance.instanceId)) break;
    }
    sequence.displayName = stateName; sequence.durationMs = 2000;
    WORLD_SEQUENCE_TRACK track; track.slotId = "object"; track.keys.push_back({});
    WORLD_SEQUENCE_TRANSFORM_KEY end; end.timeMs = sequence.durationMs; track.keys.push_back(end);
    sequence.tracks.push_back(track);
    instance.templateId = sequence.sequenceId;
    instance.anchorKind = resource.anchorKind;
    instance.bindings.push_back({"object", WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE, resource.objectId});
    if (instance.anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level)
        { m_Status = "Map state creation requires the active KoukuSaydon arena."; return false; }
        if (!level->Try_Get_AuthoringPreviewPlacement(instance.position, m_Status))
        {
            m_Status = "Map state needs the current character placement: " + m_Status;
            return false;
        }
    }
    return true;
}

void CWorldObjectTool::Change_ResourceAnchor(
    WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind)
{
    if (resource.anchorKind == anchorKind || !resource.sequenceInstanceId.empty()) return;
    float3_t position{};
    if (anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level || !level->Try_Get_AuthoringPreviewPlacement(position, m_Status))
        { m_Status = "Map anchor needs the current character placement: " + m_Status; return; }
    }
    resource.anchorKind = anchorKind;
    if (anchorKind != "BOSS") { resource.anchorBossArchetypeId.clear(); resource.anchorBone.clear(); }
    for (const auto& id : StateIds(resource))
    {
        auto* instance = m_Document.Find_Instance(id);
        if (!instance || instance->anchorKind == anchorKind) continue;
        instance->anchorKind = anchorKind;
        instance->position = position;
    }
    Mark_Dirty();
    m_Status = anchorKind == "BOSS" ?
        "Boss anchor applied to this resource's states. Choose the boss and BODY bone below." : anchorKind == "PLAYER" ?
        "Character anchor applied to this resource's states; offsets start at the character origin." :
        "Map anchor applied to this resource's states at the current character position.";
}

void CWorldObjectTool::Render()
{
    if (!m_Open) return;
    const auto* viewport = ImGui::GetMainViewport();
    const ImVec2 origin = viewport ? viewport->WorkPos : ImVec2(0.f, 0.f);
    const ImVec2 available = viewport ? viewport->WorkSize : ImVec2(1600.f, 900.f);
    constexpr float margin = 8.f, gap = 8.f;
    const float width = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
    const float height = (std::max)(1.f, available.y - margin * 2.f);
    const float leftWidth = width * .23f, rightWidth = width * .24f;
    const float centerWidth = width - leftWidth - rightWidth;
    const float leftX = origin.x + margin, centerX = leftX + leftWidth + gap;
    const float rightX = centerX + centerWidth + gap, topY = origin.y + margin;
    const ImGuiCond condition = m_ResetLayoutRequested ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    m_ResetLayoutRequested = false;
    const auto beginPane = [&](const char* name, bool& visible, const ImVec2 position, const ImVec2 size)
    {
        ImGui::SetNextWindowPos(position, condition);
        ImGui::SetNextWindowSize(size, condition);
        const bool expanded = ImGui::Begin(name, &visible, ImGuiWindowFlags_MenuBar);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsMouseClicked(0))
            m_InteractionRequested = true;
        if (expanded) Render_WindowMenu();
        return expanded;
    };

    if (m_ResourcesOpen)
    {
        if (beginPane("Object Resources###WorldObjectResourcesWindow", m_ResourcesOpen,
            {leftX, topY}, {leftWidth, height}))
        {
            if (m_Ready) Render_Resources();
            else
            {
                if (ImGui::Button("Reload Source")) Load_Source();
                ImGui::TextWrapped("%s", m_Status.c_str());
            }
            if (m_Ready) Render_EffectResources();
            Render_PhysicalResources();
            if (m_Ready) Render_AnimationResources();
        }
        ImGui::End();
    }
    if (m_SequencerOpen)
    {
        if (beginPane("Object Sequencer###WorldObjectSequencerWindow", m_SequencerOpen,
            {centerX, topY + height * .57f}, {centerWidth, height * .43f}))
        {
            Render_Toolbar();
            auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (const auto* group = Preview_Group()) Render_GroupSequence(*group);
            else if (sequence) Render_Sequence(*sequence);
            else
            {
                ImGui::TextWrapped("Select a Motion beneath an Object to open its Lifetime timeline. The parent Object edits shared resources only.");
                ImGui::BeginDisabled(); ImGui::Button("Play"); ImGui::SameLine(); ImGui::Button("Append Clip"); ImGui::EndDisabled();
            }
        }
        ImGui::End();
    }
    if (m_DetailOpen)
    {
        if (beginPane("Object Detail###WorldObjectDetailWindow", m_DetailOpen,
            {rightX, topY}, {rightWidth, height}))
        {
            if (m_Ready) Render_Detail();
            else ImGui::TextDisabled("Load Object Resources to edit an object.");
        }
        ImGui::End();
    }
    if (!m_ResourcesOpen && !m_SequencerOpen && !m_DetailOpen) m_Open = false;
    if (!m_Open) Stop_Preview();
}

void CWorldObjectTool::Render_WindowMenu()
{
    if (!ImGui::BeginMenuBar()) return;
    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Object Resources", nullptr, &m_ResourcesOpen);
        ImGui::MenuItem("Object Sequencer", nullptr, &m_SequencerOpen);
        ImGui::MenuItem("Object Detail", nullptr, &m_DetailOpen);
        ImGui::Separator();
        if (ImGui::MenuItem("Show All")) m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
        if (ImGui::MenuItem("Reset Window Layout"))
        {
            m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
            m_ResetLayoutRequested = true;
        }
        if (ImGui::MenuItem("Close World Object Tool")) m_Open = false;
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void CWorldObjectTool::Render_Toolbar()
{
    if (ImGui::Button("Reload Source"))
    {
        if (m_Dirty) ImGui::OpenPopup("Reload object source?");
        else Load_Source();
    }
    if (ImGui::BeginPopupModal("Reload object source?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Reload discards this tool's unsaved object/state edits.");
        if (ImGui::Button("Discard and Reload")) { Load_Source(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("Keep Editing")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::SameLine(); ImGui::BeginDisabled(!m_Ready || m_PublishProcess);
    if (ImGui::Button(m_Dirty ? "Save *" : "Save")) Save_Source();
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
}

void CWorldObjectTool::Render_Resources()
{
    if (ImGui::Button("Create Object"))
    {
        m_CreateObjectFailed = false;
        ImGui::OpenPopup("Create Object Resource");
    }
    ImGui::SameLine(); ImGui::TextDisabled("%zu resources", m_Document.Get_ObjectResources().size());
    if (ImGui::BeginPopupModal("Create Object Resource", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputTextWithHint("Name", "New object name", m_NewObjectName.data(), m_NewObjectName.size());
        ImGui::Combo("Anchor Type", &m_NewObjectAnchor, "Map\0Character\0Boss\0");
        ImGui::TextUnformatted(m_NewObjectAnchor == 2 ?
            "Boss: motions follow the chosen boss BODY bone. Select the boss and bone in Object Detail." : m_NewObjectAnchor == 0 ?
            "Map: motions created later use a fixed world anchor." :
            "Character: motions created later use each living character as their anchor.");
        ImGui::TextUnformatted("Creates the parent Object only. Add its motions from Object Detail.");
        ImGui::BeginDisabled(!m_NewObjectName[0]);
        if (ImGui::Button("Create"))
        {
            m_CreateObjectFailed = !Create_Object();
            if (!m_CreateObjectFailed) ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled(); ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        if (m_CreateObjectFailed) ImGui::TextWrapped("%s", m_Status.c_str());
        ImGui::EndPopup();
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectSearch", "Search object or motion", m_ObjectSearch.data(), m_ObjectSearch.size());
    const float treeHeight = (std::max)(120.f, ImGui::GetContentRegionAvail().y * .28f);
    if (ImGui::BeginChild("ObjectResourceTree", ImVec2(0.f, treeHeight), true))
    {
        const auto search = Lower(m_ObjectSearch.data());
        const auto stateMatches = [&search](const std::string& id, const WORLD_SEQUENCE_TEMPLATE* sequence)
        {
            const auto searchable = sequence ? sequence->displayName + " " + sequence->sequenceId + " " + id : id;
            return Lower(searchable).find(search) != std::string::npos;
        };
        for (const char* anchor : {"WORLD", "PLAYER", "BOSS"})
        {
            const char* category = std::string(anchor) == "WORLD" ? "Map" : std::string(anchor) == "BOSS" ? "Boss" : "Character";
            size_t count = 0;
            for (const auto& resource : m_Document.Get_ObjectResources()) if (resource.anchorKind == anchor) ++count;
            const std::string categoryLabel = std::string(category) + " (" + std::to_string(count) + ")";
            if (!ImGui::TreeNodeEx(anchor, ImGuiTreeNodeFlags_DefaultOpen, "%s", categoryLabel.c_str())) continue;
            for (const auto& resource : m_Document.Get_ObjectResources())
            {
                if (resource.anchorKind != anchor) continue;
                const auto states = StateIds(resource);
                const bool resourceMatches = search.empty() || Lower(resource.displayName + " " + resource.objectId).find(search) != std::string::npos;
                bool matches = resourceMatches;
                if (!matches)
                    for (const auto& id : states)
                    {
                        const auto* state = m_Document.Find_Instance(id);
                        const auto* sequence = state ? m_Document.Find_Template(state->templateId) : nullptr;
                        if (stateMatches(id, sequence)) { matches = true; break; }
                    }
                if (!matches) continue;
                ImGui::PushID(resource.objectId.c_str());
                if (!resource.motionInstanceIds.empty())
                {
                    // One authoring entry opens every member; it is not a folder of solo previews.
                    if (ImGui::Selectable(resource.displayName.c_str(), m_SelectedGroup == resource.objectId))
                    {
                        Select_Object(resource.objectId);
                        m_SequencerOpen = true;
                        m_DetailOpen = true;
                        Seek(0.f);
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Open all %zu motions together in Object Sequencer.", states.size());
                    ImGui::PopID();
                    continue;
                }
                const auto label = resource.displayName +
                    (resource.sequenceInstanceId.empty() && resource.modelAssetId.empty() ? " [assign model]" : "");
                const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_SpanAvailWidth | (m_SelectedObject == resource.objectId && m_SelectedInstance.empty() ? ImGuiTreeNodeFlags_Selected : 0);
                if (!search.empty()) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                const bool open = ImGui::TreeNodeEx("Resource", flags, "%s", label.c_str());
                if (ImGui::IsItemClicked()) Select_Object(resource.objectId);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", resource.objectId.c_str());
                if (open)
                {
                    for (const auto& id : states)
                    {
                        const auto* instance = m_Document.Find_Instance(id);
                        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
                        if (!resourceMatches && !stateMatches(id, sequence)) continue;
                        ImGui::PushID(id.c_str());
                        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), id == m_SelectedInstance))
                        {
                            m_SelectedGroup.clear();
                            m_SelectedObject = resource.objectId;
                            Select_State(id);
                        }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
                        ImGui::PopID();
                    }
                    if (states.empty()) ImGui::TextDisabled("No motions - select Object to Create Motion");
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_EffectResources()
{
    if (!ImGui::CollapsingHeader("V2 Effects", ImGuiTreeNodeFlags_DefaultOpen)) return;
    const bool reload = ImGui::Button("Reload V2 Effects");
    if (!m_EffectInventoryLoaded || reload)
    {
        std::vector<EFFECT_V2_RESOURCE_SUMMARY> staged;
        if (CEffectV2Catalog::Get().Read_Inventory(staged, m_EffectResourceStatus))
            m_EffectResources = std::move(staged);
        m_EffectInventoryLoaded = true;
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectEffectSearch", "Search smoke / group / leaf", m_EffectSearch.data(), m_EffectSearch.size());
    const auto search = Lower(m_EffectSearch.data());
    if (ImGui::BeginChild("ObjectV2Effects", ImVec2(0.f, 135.f), true))
        for (const auto& effect : m_EffectResources)
        {
            if (!search.empty() && Lower(effect.strDisplayName + " " + effect.strResourceId).find(search) == std::string::npos) continue;
            ImGui::PushID(effect.strResourceId.c_str());
            const auto label = std::string(effect.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "[Group] " : "[Leaf] ") +
                (effect.strDisplayName.empty() ? effect.strResourceId : effect.strDisplayName);
            ImGui::BeginDisabled(!effect.strStatus.empty());
            if (ImGui::Selectable(label.c_str(), m_SelectedEffectResource == effect.strResourceId && m_SelectedEffectKind == effect.eKind))
            { m_SelectedEffectResource = effect.strResourceId; m_SelectedEffectKind = effect.eKind; }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("%s\n%s", effect.strResourceId.c_str(), effect.strStatus.c_str());
            ImGui::PopID();
        }
    ImGui::EndChild();
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const bool target = instance && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
    ImGui::BeginDisabled(!target || m_SelectedEffectResource.empty());
    if (ImGui::Button("Append Effect at Motion End")) Append_SelectedEffect();
    ImGui::EndDisabled();
    if (!target) ImGui::TextWrapped("Select an Object's child Motion, then append an Effect row.");
    if (!m_EffectResourceStatus.empty()) ImGui::TextWrapped("%s", m_EffectResourceStatus.c_str());
}

bool CWorldObjectTool::Append_SelectedEffect()
{
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence || instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
    { m_Status = "Select a child Object Motion before appending an Effect."; return false; }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
    if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(m_SelectedEffectKind, m_SelectedEffectResource, snapshot, m_EffectResourceStatus))
    { m_Status = "Effect Append failed: " + m_EffectResourceStatus; return false; }
    // Match the existing V2 authoring preview span, including particles/trails
    // remaining after emission ends. The row does not stretch leaf envelopes.
    const auto leafSpan = [](const EFFECT_V2_DOCUMENT& document, const uint32_t explicitMs, const bool tailEnabled)
    {
        const auto& params = document.Desc.Params;
        const double rate = (std::max)(.001, static_cast<double>(params.fPlayRate));
        const double emission = explicitMs ? explicitMs : params.fLifetime > 0.f ?
            std::ceil(params.fLifetime * 1000.0 / rate) : 3000.0;
        const double tail = !tailEnabled ? 0.0 : document.eType == EFFECT_V2_TYPE::PARTICLE ?
            params.Particle.vLifetime.y * 1000.0 / rate : document.eType == EFFECT_V2_TYPE::TRAIL ?
            params.Trail.fPointLifetime * 1000.0 / rate : 0.0;
        return emission + std::ceil(tail);
    };
    double span = 0.;
    if (m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP)
    {
        const auto* group = snapshot->Find_Group(m_SelectedEffectResource);
        if (!group) { m_Status = "Selected Effect group is unavailable."; return false; }
        span = group->iDurationMs;
        if (!group->iDurationMs)
            for (const auto& child : group->Children)
            {
                const auto* leaf = snapshot->Find_Document(child.strEffectId);
                if (!leaf) { m_Status = "Selected Effect group child is unavailable."; return false; }
                span = (std::max)(span, child.iStartMs + leafSpan(*leaf, child.iDurationMs,
                    child.eStop == EFFECT_V2_CHILD_STOP::DEACTIVATE));
            }
    }
    else
    {
        const auto* leaf = snapshot->Find_Document(m_SelectedEffectResource);
        if (!leaf) { m_Status = "Selected Effect leaf is unavailable."; return false; }
        span = leafSpan(*leaf, 0u, true);
    }
    CWorldSequenceDocument staged = m_Document;
    auto* edited = staged.Find_Template(sequence->sequenceId);
    WORLD_SEQUENCE_EFFECT_TRACK row;
    uint32_t serial = 1u;
    do { row.effectTrackId = "effect." + std::to_string(serial++); }
    while (std::any_of(edited->effectTracks.begin(), edited->effectTracks.end(),
        [&](const auto& value) { return value.effectTrackId == row.effectTrackId; }));
    row.slotId = instance->bindings.front().slotId;
    row.resourceKind = m_SelectedEffectKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF";
    row.resourceId = m_SelectedEffectResource;
    row.durationMs = static_cast<uint32_t>((std::clamp)(std::ceil(span), 1., 600000.));
    edited->effectTracks.push_back(row);
    if (!staged.Validate(m_MapTargets, m_DeployTargets, m_Status)) return false;
    m_SelectedEffectRow = edited->effectTracks.size() - 1u;
    m_Document = std::move(staged);
    Mark_Dirty();
    m_Status = "Effect row appended at Motion End. Adjust it in Effect Rows, then Save.";
    return true;
}

void CWorldObjectTool::Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!ImGui::CollapsingHeader("Effect Rows", ImGuiTreeNodeFlags_DefaultOpen)) return;
    if (sequence.effectTracks.empty())
    { ImGui::TextWrapped("Choose a V2 Group or Leaf in Object Resources and Append Effect at Motion End."); return; }
    m_SelectedEffectRow = (std::min)(m_SelectedEffectRow, sequence.effectTracks.size() - 1u);
    for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
    {
        const auto& row = sequence.effectTracks[index];
        const auto label = row.resourceId + "##" + row.effectTrackId;
        if (ImGui::Selectable(label.c_str(), m_SelectedEffectRow == index)) m_SelectedEffectRow = index;
    }
    auto& row = sequence.effectTracks[m_SelectedEffectRow];
    bool changed = false;
    int timing = row.timing == "MOTION_END" ? 0 : 1;
    if (ImGui::Combo("Effect Trigger", &timing, "Motion End\0At Time\0"))
    {
        row.timing = timing == 0 ? "MOTION_END" : "TIME";
        row.startMs = timing == 0 ? 0u : sequence.durationMs;
        changed = true;
    }
    if (timing == 1) changed |= EditUInt("Effect Start (ms)", row.startMs, sequence.durationMs);
    else ImGui::TextDisabled("Follows Motion Lifetime: %u ms", sequence.durationMs);
    changed |= EditUInt("Effect Window (ms)", row.durationMs, CWorldSequenceDocument::MAX_DURATION_MS, 1);
    changed |= ImGui::DragFloat3("Effect Offset (m)", &row.positionOffset.x, .01f);
    changed |= ImGui::DragFloat3("Effect Rotation (deg)", &row.rotationDegrees.x, .5f);
    changed |= ImGui::DragFloat3("Effect Scale", &row.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextWrapped("Each emitted object triggers this Effect at its own trajectory position. The Effect stays there after the model ends; its original envelope is preserved.");
    if (ImGui::Button("Remove Selected Effect Row"))
    {
        sequence.effectTracks.erase(sequence.effectTracks.begin() + static_cast<ptrdiff_t>(m_SelectedEffectRow));
        m_SelectedEffectRow = 0;
        changed = true;
    }
    if (changed) Mark_Dirty();
}


void CWorldObjectTool::Refresh_AnimationResources()
{
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
    {
        m_AnimationResources.clear();
        m_SelectedAnimationClip.clear();
    }
    m_AnimationObjectId = m_SelectedObject;
    m_AnimationModelAssetId = modelAssetId;
    m_AnimationCatalogReady = false;
    if (!resource)
    { m_AnimationResourceStatus = "Select or Create Object first."; return; }
    if (!resource->sequenceInstanceId.empty())
    { m_AnimationResourceStatus = "This placed object uses its existing sequence tracks; it has no separate native clip catalog."; return; }
    if (modelAssetId.empty())
    { m_AnimationResourceStatus = "Select a WModel in Physical Resources to list its native animations."; return; }
    const auto path = CRuntimeAssetRoot::Resolve(modelAssetId);
    std::vector<Engine::MODEL_ANIMATION_CATALOG_ENTRY> catalog;
    std::string status;
    if (path.empty() || !Engine::CWModelDecoder::Read_AnimationCatalog(path, catalog, status))
    {
        m_AnimationResourceStatus = "Animation catalog unavailable: " + modelAssetId + ": " +
            (path.empty() ? "invalid Resources-relative path" : status) + ". Existing object and patterns are unchanged.";
        return;
    }
    std::vector<ANIMATION_RESOURCE> staged;
    for (const auto& clip : catalog)
    {
        // Match the CAnimation clock used by WorldSequenceObject::Sample.
        const double duration = static_cast<double>(clip.durationTicks) / Engine::CAnimation::COOKED_TICK_RATE * 1000.;
        if (clip.name.empty() || !std::isfinite(duration) || duration <= 0.)
        { m_AnimationResourceStatus = "Invalid animation timing: " + modelAssetId + ". Existing patterns are unchanged."; return; }
        staged.push_back({clip.name, duration});
    }
    m_AnimationResources = std::move(staged);
    m_AnimationCatalogReady = true;
    if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == m_SelectedAnimationClip;
    })) m_SelectedAnimationClip.clear();
    m_AnimationResourceStatus = m_AnimationResources.empty() ?
        "This model has no native animation. Assign Model on the parent Object, then author Transform keys or Physics on a child Motion." :
        std::to_string(m_AnimationResources.size()) + " native clips. Select one and Append Clip to the selected Motion.";
}

bool CWorldObjectTool::Stage_SelectedModel(CWorldSequenceDocument& candidate)
{
    auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || !resource->sequenceInstanceId.empty() || !m_AnimationCatalogReady ||
        m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId.empty())
    { m_Status = "Select an available physical WModel for this object first."; return false; }
    // A resource can own several saved patterns. Keep all of their clip bindings valid.
    for (const auto& instance : candidate.Get_Instances())
    {
        const auto* sequence = candidate.Find_Template(instance.templateId);
        if (!sequence) continue;
        for (const auto& binding : instance.bindings)
        {
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource->objectId) continue;
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId) continue;
                if (std::none_of(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
                    return clip.clipName == animation.clipName;
                }))
                {
                    m_Status = "Model change refused: pattern '" + sequence->displayName + "' uses '" + animation.clipName +
                        "', which is absent from " + m_AnimationModelAssetId + ". Existing model and patterns are unchanged.";
                    return false;
                }
            }
        }
    }
    resource->modelAssetId = m_AnimationModelAssetId;
    resource->animated = !m_AnimationResources.empty();
    if (!resource->materialSourceModelAssetId.empty())
    {
        Engine::MODEL_ASSET_LOAD_DESC load;
        if (!CActorCatalog::Build_DerivedModelLoadDescription(resource->modelAssetId,
            resource->materialSourceModelAssetId, load, m_Status))
        { m_Status = "Model change refused: " + m_Status + ". Existing model and patterns preserved."; return false; }
    }
    return true;
}

bool CWorldObjectTool::Assign_SelectedModel()
{
    if (!m_SelectedInstance.empty())
    { m_Status = "Select the parent Object to assign its shared model."; return false; }
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    CWorldSequenceDocument candidate = m_Document;
    if (!Stage_SelectedModel(candidate)) return false;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Model assignment refused: " + status + ". Existing draft preserved."; return false; }
    const auto pristinePattern = m_PristinePatternId;
    m_Document = std::move(candidate);
    Mark_Dirty();
    // Assigning a model does not edit the newly created pattern's default timing.
    m_PristinePatternId = pristinePattern;
    m_Status = "Shared model assigned. Create or select a Motion, then Append Clip or edit Transform/Physics.";
    return true;
}

bool CWorldObjectTool::Append_SelectedAnimation()
{
    const auto selectedClip = m_SelectedAnimationClip;
    Refresh_AnimationResources();
    if (!m_AnimationCatalogReady) { m_Status = m_AnimationResourceStatus; return false; }
    const auto found = std::find_if(m_AnimationResources.begin(), m_AnimationResources.end(), [&](const auto& clip) {
        return clip.clipName == selectedClip;
    });
    if (selectedClip.empty() || found == m_AnimationResources.end())
    { m_Status = "Select an available native animation first; the current object pattern is unchanged."; return false; }
    const double nativeMs = std::ceil(found->durationMs);
    if (!std::isfinite(nativeMs) || nativeMs < 1. || nativeMs > CWorldSequenceDocument::MAX_DURATION_MS)
    { m_Status = "The selected animation exceeds the supported 600-second pattern lifetime."; return false; }
    const uint32_t clipMs = static_cast<uint32_t>(nativeMs);
    CWorldSequenceDocument candidate = m_Document;
    auto* instance = candidate.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? candidate.Find_Template(instance->templateId) : nullptr;
    if (!sequence)
    { m_Status = "Select or create a Motion before appending a clip."; return false; }
    const auto* resource = candidate.Find_ObjectResource(m_SelectedObject);
    if (!resource || resource->modelAssetId != m_AnimationModelAssetId || !resource->animated)
    { m_Status = "Assign the animated model on the parent Object before appending its clips."; return false; }
    std::string slotId;
    for (const auto& binding : instance->bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != m_SelectedObject) continue;
        if (!slotId.empty())
        { m_Status = "This pattern has multiple bindings for the selected object; choose a single-object pattern."; return false; }
        slotId = binding.slotId;
    }
    if (slotId.empty())
    { m_Status = "The selected pattern does not bind the selected object. Existing draft preserved."; return false; }
    if (sequence->tracks.size() + sequence->animationTracks.size() + sequence->effectTracks.size() >= CWorldSequenceDocument::MAX_TRACK_COUNT)
    { m_Status = "The pattern has reached its track limit. Existing draft preserved."; return false; }
    const bool firstOfSlot = std::none_of(sequence->animationTracks.begin(), sequence->animationTracks.end(), [&](const auto& clip) {
        return clip.slotId == slotId;
    });
    const bool pristine = firstOfSlot && sequence->animationTracks.empty() && m_PristinePatternId == m_SelectedInstance;
    const uint32_t oldDuration = sequence->durationMs;
    const uint32_t startMs = firstOfSlot ? 0u : oldDuration;
    if (startMs > CWorldSequenceDocument::MAX_DURATION_MS - clipMs)
    { m_Status = "Appending this clip would exceed the 600-second pattern lifetime. Existing draft preserved."; return false; }
    const uint32_t duration = pristine ? clipMs : (std::max)(oldDuration, startMs + clipMs);
    for (auto& track : sequence->tracks)
    {
        if (track.keys.empty())
        { m_Status = "The pattern has an empty Transform track. Existing draft preserved."; return false; }
        if (pristine)
            track.keys.back().timeMs = duration;
        else if (duration > oldDuration)
        {
            if (track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT)
            { m_Status = "Extending this pattern would exceed its Transform key limit. Existing draft preserved."; return false; }
            // Preserve every authored time; extend only the final held pose.
            auto endpoint = track.keys.back(); endpoint.timeMs = duration;
            track.keys.push_back(endpoint);
        }
    }
    WORLD_SEQUENCE_ANIMATION_TRACK animation;
    animation.slotId = slotId; animation.clipName = selectedClip; animation.startMs = startMs;
    animation.playbackRate = 1.f; animation.loop = false; animation.holdLastFrame = true;
    sequence->animationTracks.push_back(std::move(animation));
    sequence->durationMs = duration;
    std::string status;
    if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
    { m_Status = "Animation append refused: " + status + ". Existing draft preserved."; return false; }
    m_Document = std::move(candidate);
    Mark_Dirty();
    m_Status = "Appended " + selectedClip + " at " + std::to_string(startMs) + " ms. Play in Object Sequencer, tune in Object Detail, then Save.";
    return true;
}

void CWorldObjectTool::Render_AnimationResources()
{
    ImGui::SeparatorText("Animation Resources");
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const std::string modelAssetId = !resource || !resource->sequenceInstanceId.empty() ? "" :
        (m_SelectedInstance.empty() && m_AnimationCandidateObjectId == m_SelectedObject && !m_AnimationCandidateModelAssetId.empty() ?
            m_AnimationCandidateModelAssetId : resource->modelAssetId);
    if (m_AnimationObjectId != m_SelectedObject || m_AnimationModelAssetId != modelAssetId)
        Refresh_AnimationResources();
    if (!resource || !resource->sequenceInstanceId.empty())
    { ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str()); return; }
    if (ImGui::Button("Refresh Animations")) Refresh_AnimationResources();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_SelectedInstance.empty() || !m_AnimationCatalogReady || m_AnimationModelAssetId.empty());
    if (ImGui::Button("Assign Model")) { Assign_SelectedModel(); resource = m_Document.Find_ObjectResource(m_SelectedObject); }
    ImGui::EndDisabled();
    ImGui::TextWrapped("%s", m_AnimationResourceStatus.c_str());
    if (!modelAssetId.empty()) ImGui::TextWrapped("Model: %s", modelAssetId.c_str());
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (sequence) ImGui::TextWrapped("Selected Motion: %s", sequence->displayName.c_str());
    else ImGui::TextWrapped("Select a child Motion to append clips. Shared model assignment belongs to the parent Object.");
    ImGui::BeginDisabled(!m_AnimationCatalogReady || m_SelectedAnimationClip.empty() || !sequence);
    if (ImGui::Button("Append Clip")) Append_SelectedAnimation();
    ImGui::EndDisabled();
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##AnimationSearch", "Search native animation", m_AnimationSearch.data(), m_AnimationSearch.size());
    const auto search = Lower(m_AnimationSearch.data());
    const float height = (std::max)(100.f, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("NativeAnimationCatalog", ImVec2(0.f, height), true))
    {
        for (const auto& clip : m_AnimationResources)
        {
            if (!search.empty() && Lower(clip.clipName).find(search) == std::string::npos) continue;
            ImGui::PushID(clip.clipName.c_str());
            const float nameWidth = (std::max)(80.f, ImGui::GetContentRegionAvail().x - 82.f);
            if (ImGui::Selectable(clip.clipName.c_str(), clip.clipName == m_SelectedAnimationClip, 0, ImVec2(nameWidth, 0.f)))
                m_SelectedAnimationClip = clip.clipName;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%.3f ms\n%s", clip.clipName.c_str(), clip.durationMs, m_AnimationModelAssetId.c_str());
            ImGui::SameLine(); ImGui::TextDisabled("%.0f ms", clip.durationMs);
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText("Object / Shared Resources");
    bool changed = EditText("Object Name", resource.displayName);
    ImGui::TextDisabled("%s", resource.objectId.c_str());
    const bool alias = !resource.sequenceInstanceId.empty();
    int resourceAnchor = resource.anchorKind == "BOSS" ? 2 : resource.anchorKind == "PLAYER" ? 1 : 0;
    ImGui::BeginDisabled(alias);
    if (ImGui::Combo("Anchor Type", &resourceAnchor, "Map\0Character\0Boss\0"))
        Change_ResourceAnchor(resource, resourceAnchor == 2 ? "BOSS" : resourceAnchor == 1 ? "PLAYER" : "WORLD");
    ImGui::EndDisabled();
    if (resource.anchorKind == "BOSS")
    {
        static constexpr const char* actorIds[] = {
            "BOSS_KAKULSAYDON_G1_KOUKU", "BOSS_KAKULSAYDON_G1_SAYDON",
            "BOSS_KAKULSAYDON_G2_KOUKU", "BOSS_KAKULSAYDON_G2_BIG_SAYDON", "BOSS_KAKULSAYDON_G3_SAYDON"};
        static constexpr const char* actorLabels[] = {
            "Kouku / Gate 1", "Saydon / Gate 1", "Kouku / Gate 2", "Big Saydon / Gate 2", "Saydon / Gate 3"};
        const char* selected = resource.anchorBossArchetypeId.empty() ? "Choose Boss" : resource.anchorBossArchetypeId.c_str();
        for (size_t i = 0u; i < std::size(actorIds); ++i)
            if (resource.anchorBossArchetypeId == actorIds[i]) selected = actorLabels[i];
        if (ImGui::BeginCombo("Boss Actor", selected))
        {
            for (size_t i = 0u; i < std::size(actorIds); ++i)
                if (ImGui::Selectable(actorLabels[i], resource.anchorBossArchetypeId == actorIds[i]))
                { resource.anchorBossArchetypeId = actorIds[i]; changed = true; }
            ImGui::EndCombo();
        }
        changed |= EditText("Boss Archetype ID", resource.anchorBossArchetypeId);
        changed |= EditText("BODY Bone", resource.anchorBone);
        ImGui::TextWrapped("Follows the named BODY bone every frame. Empty bone uses the boss root. Hand props use b_wp_1 or b_wp_2. Transform keys edit the local grip offset and rotation.");
    }
    if (alias) ImGui::TextDisabled("Placed objects keep their Map anchor.");
    if (alias)
        ImGui::TextWrapped("Placed object sequence: %s. This editor updates its existing tracks and bindings.", resource.sequenceInstanceId.c_str());
    else
    {
        ImGui::TextWrapped("Model: %s", resource.modelAssetId.empty() ? "Choose a WModel in Physical Resources" : resource.modelAssetId.c_str());
        if (m_MaterialSourceObjectId != resource.objectId)
        {
            m_MaterialSourceObjectId = resource.objectId;
            m_MaterialSourceCandidate = resource.materialSourceModelAssetId;
            m_MaterialSourceStatus.clear();
        }
        EditText("Material Source Model", m_MaterialSourceCandidate, 512);
        ImGui::TextWrapped("For an extracted or baked variant, enter the original catalog model path. This preserves its own textures, surface values and reflection inputs. Empty uses this model's own materials.");
        ImGui::BeginDisabled(resource.modelAssetId.empty());
        if (ImGui::SmallButton("Apply Material Source"))
        {
            Engine::MODEL_ASSET_LOAD_DESC load;
            if (CActorCatalog::Build_DerivedModelLoadDescription(resource.modelAssetId,
                m_MaterialSourceCandidate, load, m_MaterialSourceStatus))
            {
                resource.materialSourceModelAssetId = m_MaterialSourceCandidate;
                m_MaterialSourceStatus = load.materialOverrides.empty() ?
                    "Using embedded model materials. No source catalog override is registered." :
                    "Original catalog materials applied: " + std::to_string(load.materialOverrides.size()) + " slots. Save to keep this binding.";
                changed = true;
            }
            else m_MaterialSourceStatus += ". Existing material binding preserved.";
        }
        ImGui::EndDisabled();
        if (!m_MaterialSourceStatus.empty()) ImGui::TextWrapped("%s", m_MaterialSourceStatus.c_str());
        else ImGui::TextWrapped("Current material source: %s", resource.materialSourceModelAssetId.empty() ?
            "Own model / embedded materials" : resource.materialSourceModelAssetId.c_str());
        ImGui::TextWrapped("Diffuse: %s", resource.diffuseTextureAssetId.empty() ? "Embedded model material" : resource.diffuseTextureAssetId.c_str());
        if (!resource.diffuseTextureAssetId.empty() && ImGui::SmallButton("Clear Diffuse Override")) { resource.diffuseTextureAssetId.clear(); changed = true; }
        changed |= ImGui::DragFloat("Model Import Scale", &resource.modelPreScale, .001f, .000001f, 1000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Object Scale", &resource.scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::Checkbox("Animated Model", &resource.animated);
    }
    if (changed) Mark_Dirty();
    if (!alias)
    {
        const auto* initial = m_Document.Find_Instance(resource.defaultMotionInstanceId);
        const auto* initialSequence = initial ? m_Document.Find_Template(initial->templateId) : nullptr;
        if (initialSequence && !initialSequence->tracks.empty() && !initialSequence->tracks.front().keys.empty())
        {
            const auto& first = initialSequence->tracks.front().keys.front();
            auto position = first.positionOffset;
            auto rotation = QuaternionEuler(first.rotationQuaternion);
            ImGui::SeparatorText("Object Transform / All Motion Keys");
            bool transformChanged = ImGui::DragFloat3("Object Position (m)", &position.x, .01f);
            transformChanged |= ImGui::DragFloat3("Object Rotation (deg)", &rotation.x, .25f);
            ImGui::TextWrapped("Position and Rotation adjust every connected Motion key relative to its current pose. Object Scale above applies to all Motions. Use a child Motion for individual keys.");
            if (transformChanged)
            {
                auto candidate = m_Document;
                const auto translationDelta = XMLoadFloat3(&position) - XMLoadFloat3(&first.positionOffset);
                const auto rotationDelta = XMMatrixTranspose(XMMatrixRotationQuaternion(XMLoadFloat4(&first.rotationQuaternion))) *
                    XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
                std::vector<std::string> editedTemplates;
                for (const auto& id : StateIds(resource))
                {
                    const auto* motion = candidate.Find_Instance(id);
                    auto* sequence = motion ? candidate.Find_Template(motion->templateId) : nullptr;
                    if (!sequence || std::find(editedTemplates.begin(), editedTemplates.end(), sequence->sequenceId) != editedTemplates.end()) continue;
                    // Shared templates cannot be rewritten on behalf of a different Object.
                    for (const auto& other : candidate.Get_Instances())
                        if (other.templateId == sequence->sequenceId)
                            for (const auto& binding : other.bindings)
                                if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE || binding.targetId != resource.objectId)
                                { m_Status = "This Motion template is shared with another target. Edit its keys separately; Object transform was preserved."; return; }
                    for (auto& track : sequence->tracks)
                        for (auto& key : track.keys)
                        {
                            XMStoreFloat3(&key.positionOffset, XMLoadFloat3(&key.positionOffset) + translationDelta);
                            XMStoreFloat4(&key.rotationQuaternion, XMQuaternionNormalize(XMQuaternionRotationMatrix(
                                XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) * rotationDelta)));
                        }
                    editedTemplates.push_back(sequence->sequenceId);
                }
                std::string status;
                if (!candidate.Validate(m_MapTargets, m_DeployTargets, status))
                { m_Status = "Object transform refused: " + status + ". Existing draft preserved."; return; }
                m_Document = std::move(candidate);
                Mark_Dirty();
                m_Status = "Updated the Object transform in every connected Motion. Save preserves these keys.";
                return;
            }
        }
        else ImGui::TextDisabled("Choose a Default Motion to edit Object Position and Rotation here.");
    }
    ImGui::SeparatorText("Connected Motions");
    const auto motions = StateIds(resource);
    const auto* defaultMotion = m_Document.Find_Instance(resource.defaultMotionInstanceId);
    const auto* defaultSequence = defaultMotion ? m_Document.Find_Template(defaultMotion->templateId) : nullptr;
    const char* defaultLabel = defaultSequence ? defaultSequence->displayName.c_str() : "None - choose a Default Motion";
    if (ImGui::BeginCombo("Default Motion", defaultLabel))
    {
        if (ImGui::Selectable("None", resource.defaultMotionInstanceId.empty()))
        { resource.defaultMotionInstanceId.clear(); Mark_Dirty(); }
        for (const auto& id : motions)
        {
            const auto* instance = m_Document.Find_Instance(id);
            const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            ImGui::PushID(id.c_str());
            ImGui::BeginDisabled(!instance || !instance->enabled || !sequence);
            if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), resource.defaultMotionInstanceId == id))
            { resource.defaultMotionInstanceId = id; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    if (!defaultMotion || !defaultMotion->enabled)
        ImGui::TextWrapped("Choose an enabled Default Motion to Append or Preview this Object.");
    ImGui::BeginDisabled(!Preview_Instance());
    if (ImGui::Button("Preview Default")) { Stop_Preview(); m_PreviewAtCharacter = true; m_ClockMs = 0.f; Seek(0.f); m_Playing = m_PreviewActive; }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("Stop Preview")) { Stop_Preview(); m_ClockMs = 0.f; }
    if (motions.empty()) ImGui::TextDisabled("No motions yet.");
    for (const auto& id : motions)
    {
        const auto* instance = m_Document.Find_Instance(id);
        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        ImGui::PushID(id.c_str());
        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str()))
        { Select_State(id); ImGui::PopID(); return; }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
        if (sequence && instance)
        {
            ImGui::TextDisabled("%zu clips | %u ms | %s", sequence->animationTracks.size(),
                sequence->durationMs, MotionEndLabel(instance->motionEnd));
            if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
            {
                const auto* next = m_Document.Find_Instance(instance->nextMotionId);
                const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
                ImGui::TextDisabled("  Next: %s", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion");
            }
        }
        ImGui::PopID();
    }
    ImGui::BeginDisabled(alias);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##NewMotion", "Motion name", m_NewStateName.data(), m_NewStateName.size());
    ImGui::BeginDisabled(!m_NewStateName[0]);
    if (ImGui::Button("Create Motion")) Create_State();
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    if (alias) ImGui::TextWrapped("This placed Object retains its existing Motion and bindings.");
}

void CWorldObjectTool::Render_Detail()
{
    if (auto* group = m_Document.Find_ObjectResource(m_SelectedGroup)) Render_GroupDetail(*group);
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource) { ImGui::TextUnformatted("Select an object resource."); return; }
    if (m_SelectedInstance.empty()) { Render_ObjectDetail(*resource); return; }
    ImGui::TextWrapped("Object: %s", resource->displayName.c_str());
    if (!Preview_Group() && ImGui::Button("Edit Parent Object")) { Select_Object(resource->objectId); return; }
    const bool alias = !resource->sequenceInstanceId.empty();
    bool changed = false;
    auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence) { ImGui::TextWrapped("The selected Motion is unavailable. Select its parent Object to continue."); return; }
    ImGui::SeparatorText("Motion");
    changed |= EditText("Motion Name", sequence->displayName);
    ImGui::TextDisabled("%s", instance->instanceId.c_str());
    changed |= ImGui::Checkbox("Enabled", &instance->enabled);
    if (!alias)
    {
        ImGui::TextDisabled("Creation Anchor: %s", instance->anchorKind == "BOSS" ? "Boss / BODY Bone" : instance->anchorKind == "PLAYER" ? "Character" : "Map");
        changed |= ImGui::DragFloat3(instance->anchorKind == "BOSS" ? "Bone Offset" : instance->anchorKind == "PLAYER" ? "Character Offset" : "Map Position", &instance->position.x, .01f);
        if (ImGui::Button("Use Current Character Position"))
        {
            if (instance->anchorKind == "PLAYER" || instance->anchorKind == "BOSS") { instance->position = {}; changed = true; }
            else if (auto* level = CLevel_KakulSaydonArena::Get_Active()) changed |= level->Try_Get_AuthoringPreviewPlacement(instance->position, m_Status);
            else m_Status = "Player placement requires the active KoukuSaydon arena.";
        }
    }
    const bool supportsSurface = alias && instance->bindings.size() == 1u &&
        instance->bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
    if (supportsSurface || instance->walkableSurface)
    {
        ImGui::SeparatorText("Walkable Surface");
        bool enabled = instance->walkableSurface.has_value();
        if (ImGui::Checkbox("Enable Circular Walking Surface", &enabled))
        {
            if (enabled) instance->walkableSurface = WORLD_SEQUENCE_WALKABLE_SURFACE{};
            else instance->walkableSurface.reset();
            changed = true;
        }
        if (instance->walkableSurface)
        {
            changed |= ImGui::DragFloat("Surface Radius (local m)", &instance->walkableSurface->radiusM,
                .01f, .001f, 1000.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Surface Height (local m)", &instance->walkableSurface->localHeightM,
                .001f, -10000.f, 10000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextWrapped("A horizontal circle centered on the placed model. Values use meters after model import scale; placement scale is applied automatically. Fixed position/scale and Y rotation only. Server Complete Play uses its visible lifetime; blocked ground stays blocked. Save, then publish the Map and Kouku gameplay data.");
        }
    }
    changed |= EditUInt("Start Delay (ms)", instance->startDelayMs, CWorldSequenceDocument::MAX_DURATION_MS);
    changed |= ImGui::DragFloat("Playback Speed", &instance->playbackSpeed, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    uint32_t duration = sequence->durationMs;
    if (EditUInt("Lifetime (ms)", duration, CWorldSequenceDocument::MAX_DURATION_MS, 1))
    {
        // Preserve the key order and endpoint contract even when shortening a state.
        bool valid = true;
        for (const auto& track : sequence->tracks) if (duration + 1u < track.keys.size()) valid = false;
        for (const auto& animation : sequence->animationTracks)
            if (static_cast<size_t>(duration) < std::count_if(sequence->animationTracks.begin(), sequence->animationTracks.end(),
                [&](const auto& other) { return other.slotId == animation.slotId; })) valid = false;
        if (valid)
        {
            for (auto& track : sequence->tracks)
            {
                for (size_t key = 1; key + 1 < track.keys.size(); ++key)
                {
                    const auto scaled = static_cast<uint32_t>(std::llround(static_cast<double>(track.keys[key].timeMs) * duration / sequence->durationMs));
                    track.keys[key].timeMs = (std::clamp)(scaled, track.keys[key - 1].timeMs + 1,
                        duration - static_cast<uint32_t>(track.keys.size() - key - 1));
                }
                if (!track.keys.empty()) track.keys.back().timeMs = duration;
            }
            std::unordered_map<std::string, uint32_t> previousStarts;
            for (size_t index = 0; index < sequence->animationTracks.size(); ++index)
            {
                auto& animation = sequence->animationTracks[index];
                const auto previous = previousStarts.find(animation.slotId);
                const auto remaining = std::count_if(sequence->animationTracks.begin() + index + 1, sequence->animationTracks.end(),
                    [&](const auto& other) { return other.slotId == animation.slotId; });
                const uint32_t scaled = static_cast<uint32_t>(static_cast<double>(animation.startMs) * duration / sequence->durationMs);
                animation.startMs = previous == previousStarts.end() ? 0 : (std::clamp)(scaled,
                    previous->second + 1, duration - static_cast<uint32_t>(remaining) - 1);
                previousStarts[animation.slotId] = animation.startMs;
            }
            for (auto& effect : sequence->effectTracks)
                if (effect.timing == "TIME")
                    effect.startMs = static_cast<uint32_t>(static_cast<uint64_t>(effect.startMs) * duration / sequence->durationMs);
            sequence->durationMs = duration;
            if (sequence->objectMotion.count > 1)
                sequence->objectMotion.intervalMs = (std::min)(sequence->objectMotion.intervalMs, (sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration) / (sequence->objectMotion.count - 1));
            for (auto& emission : sequence->objectMotion.emissions)
                emission.startDelayMs = (std::min)(emission.startDelayMs, sequence->effectTracks.empty() ? duration - 1 : CWorldSequenceDocument::MAX_DURATION_MS - duration);
            changed = true;
        }
        else m_Status = "Lifetime must leave at least one millisecond between every existing key or clip.";
    }
    ImGui::BeginDisabled(alias || Preview_Group());
    int motionEnd = static_cast<int>(instance->motionEnd);
    if (ImGui::Combo("On Complete", &motionEnd, "Stop\0Hold Last Pose\0Loop\0Play Motion\0"))
    {
        instance->motionEnd = static_cast<WORLD_SEQUENCE_MOTION_END>(motionEnd);
        if (instance->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) instance->nextMotionId.clear();
        changed = true;
    }
    ImGui::EndDisabled();
    if (Preview_Group()) ImGui::TextDisabled("Combined preview members use Stop; each Lifetime remains editable.");
    if (alias) ImGui::TextDisabled("Placed Object motions use Stop.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
    {
        const auto* next = m_Document.Find_Instance(instance->nextMotionId);
        const auto* nextSequence = next ? m_Document.Find_Template(next->templateId) : nullptr;
        if (ImGui::BeginCombo("Next Motion", nextSequence ? nextSequence->displayName.c_str() : "Choose a Motion"))
        {
            for (const auto& id : StateIds(*resource))
            {
                const auto* candidate = m_Document.Find_Instance(id);
                const auto* candidateSequence = candidate ? m_Document.Find_Template(candidate->templateId) : nullptr;
                if (!candidateSequence || candidate->instanceId == instance->instanceId) continue;
                const bool compatible = candidate->enabled && sequence->objectMotion.count == 1u &&
                    candidateSequence->objectMotion.count == 1u && instance->bindings.size() == 1u &&
                    candidate->bindings.size() == 1u && instance->bindings.front().slotId == candidate->bindings.front().slotId;
                ImGui::PushID(id.c_str());
                ImGui::BeginDisabled(!compatible);
                if (ImGui::Selectable(candidateSequence->displayName.c_str(), instance->nextMotionId == id))
                { instance->nextMotionId = id; changed = true; }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("%s%s", id.c_str(), compatible ? "" : "\nRequires an enabled Motion with Count 1 and the same object slot.");
                ImGui::EndDisabled();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::TextWrapped("At Lifetime, continue the same object with this Motion. Each Motion in the link must use Count 1; links cannot form a cycle.");
    }
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::HOLD)
        ImGui::TextWrapped("Keep the final transform and animation pose after Lifetime.");
    else if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP)
        ImGui::TextWrapped("Repeat this Motion after Lifetime. This Loop is saved and also used by Play.");
    else ImGui::TextWrapped("Standalone Play ends the object at Lifetime. A Motion applied by a Result stops at its final pose; the target WORLD cue keeps its original lifetime.");
    int interpolation = sequence->interpolation == WORLD_SEQUENCE_INTERPOLATION::LINEAR ? 0 : 1;
    if (ImGui::Combo("Interpolation", &interpolation, "Linear\0Smooth Step\0"))
    { sequence->interpolation = interpolation == 0 ? WORLD_SEQUENCE_INTERPOLATION::LINEAR : WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP; changed = true; }
    if (!alias && ImGui::CollapsingHeader("Physics / Motion / Emission", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& motion = sequence->objectMotion;
        auto& origins = Emission_Origins(*sequence);
        ImGui::TextWrapped("Motion uses the Lifetime timeline and works without animation clips. Play or drag the ruler to preview, then Save.");
        ImGui::DragFloat("Arc Height (m)", &m_VerticalArcHeight, .05f, .01f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Apply Vertical Arc"))
        {
            const float seconds = sequence->durationMs * .001f;
            const float velocityY = 4.f * m_VerticalArcHeight / seconds;
            const float accelerationY = -8.f * m_VerticalArcHeight / (seconds * seconds);
            if (!std::isfinite(m_VerticalArcHeight) || m_VerticalArcHeight <= 0.f ||
                !std::isfinite(velocityY) || !std::isfinite(accelerationY) ||
                std::abs(velocityY) > 100000.f || std::abs(accelerationY) > 100000.f)
                m_Status = "Vertical arc is too fast for this Lifetime. Increase Lifetime or reduce Arc Height; motion preserved.";
            else
            {
                motion.velocity = {0.f, velocityY, 0.f};
                motion.acceleration = {0.f, accelerationY, 0.f};
                motion.spreadDegrees = 0.f;
                changed = true;
                m_Status = "Vertical arc applied. Count and interval preserved. First emission returns at Lifetime. Effect-bearing motions give every emission a full Lifetime. Save to keep the motion.";
            }
        }
        ImGui::TextDisabled("First emission: apex at %.0f ms; return at %u ms.", sequence->durationMs * .5f, sequence->durationMs);
        ImGui::TextWrapped("Arc Height sets the physics offset above the Transform track. With Effect rows, every emission has a full Lifetime and its own Effect tail. Apply again after changing Lifetime. Velocity and Acceleration remain editable.");
        changed |= ImGui::DragFloat3("Velocity (m/s)", &motion.velocity.x, .05f);
        changed |= ImGui::DragFloat3("Acceleration (m/s2)", &motion.acceleration.x, .05f);
        changed |= ImGui::DragFloat3("Self Rotation (deg/s)", &motion.angularVelocityDegrees.x, .5f);
        changed |= ImGui::DragFloat3("Revolution (deg/s)", &motion.revolutionDegreesPerSecond.x, .5f);
        changed |= ImGui::DragFloat3("Revolution Offset (m)", &motion.revolutionOffset.x, .05f);
        auto& emissions = motion.emissions;
        ImGui::BeginDisabled(!emissions.empty());
        if (EditUInt("Count", motion.count, 128, 1))
        {
            origins.resize(motion.count, origins.empty() ? 0u : origins.back());
            m_GroupCount = static_cast<int>(motion.count);
            changed = true;
        }
        const uint32_t maxInterval = motion.count > 1 ? (sequence->effectTracks.empty() ? sequence->durationMs - 1 :
            CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs) / (motion.count - 1) : CWorldSequenceDocument::MAX_DURATION_MS;
        if (motion.intervalMs > maxInterval) { motion.intervalMs = maxInterval; changed = true; }
        changed |= EditUInt("Creation Interval (ms)", motion.intervalMs, maxInterval);
        changed |= ImGui::DragFloat(sequence->effectTracks.empty() ? "Spread (deg)" : "Horizontal Spread (deg)", &motion.spreadDegrees, .5f, 0.f, sequence->effectTracks.empty() ? 180.f : 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndDisabled();
        changed |= ImGui::DragFloat3("Spawn Half Extents (m)", &motion.spawnHalfExtents.x, .05f, 0.f, 100000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::TextDisabled("Width / height / depth = twice these values. Set Y to 0 for a ground rectangle.");
        changed |= EditUInt("Seed", motion.seed, INT_MAX);
        ImGui::SeparatorText("Authored Emissions");
        ImGui::TextWrapped("Rows replace the seeded spread. Each row replays this Motion's keys, physics and revolution from its own local offset, yaw and delay. Leave the list empty to keep Count / Creation Interval / Spread.");
        const uint32_t maxDelay = sequence->effectTracks.empty() ? sequence->durationMs - 1 : CWorldSequenceDocument::MAX_DURATION_MS - sequence->durationMs;
        size_t removeRow = emissions.size(), duplicateRow = emissions.size();
        if (!emissions.empty() && ImGui::BeginTable("AuthoredEmissions", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 28.f);
            ImGui::TableSetupColumn("Offset X (m)");
            ImGui::TableSetupColumn("Offset Y (m)");
            ImGui::TableSetupColumn("Offset Z (m)");
            ImGui::TableSetupColumn("Yaw (deg)");
            ImGui::TableSetupColumn("Start Delay (ms)");
            ImGui::TableSetupColumn("##rowActions", ImGuiTableColumnFlags_WidthFixed, 84.f);
            ImGui::TableHeadersRow();
            for (size_t row = 0; row < emissions.size(); ++row)
            {
                auto& emission = emissions[row];
                ImGui::PushID(static_cast<int>(row));
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row);
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetX", &emission.positionOffset.x, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetY", &emission.positionOffset.y, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##offsetZ", &emission.positionOffset.z, .05f, -100000.f, 100000.f, "%.3f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::DragFloat("##yaw", &emission.yawDegrees, .5f, -36000.f, 36000.f, "%.1f");
                ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= EditUInt("##delay", emission.startDelayMs, static_cast<int>(maxDelay));
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Dup")) duplicateRow = row;
                ImGui::SameLine();
                if (ImGui::SmallButton("Del")) removeRow = row;
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        if (duplicateRow < emissions.size() && emissions.size() < 128u)
        {
            emissions.insert(emissions.begin() + duplicateRow + 1, emissions[duplicateRow]);
            origins.insert(origins.begin() + duplicateRow + 1, origins[duplicateRow]);
            m_GroupCount = static_cast<int>(emissions.size());
            changed = true;
        }
        if (removeRow < emissions.size())
        {
            if (emissions.size() == 1u) m_Status = "Keep at least one row; use Clear Emissions to return to one emitter.";
            else
            { emissions.erase(emissions.begin() + removeRow); origins.erase(origins.begin() + removeRow); m_GroupCount = static_cast<int>(emissions.size()); changed = true; }
        }
        ImGui::BeginDisabled(emissions.size() >= 128u);
        if (ImGui::Button("Add Emission"))
        {
            WORLD_SEQUENCE_OBJECT_EMISSION emission;
            if (!emissions.empty()) emission = emissions.back();
            if (emissions.empty())
            {
                // Keep the existing single emitter as row zero before adding its copy.
                emissions.push_back(emission);
                origins.resize(1u, 0u);
            }
            emissions.push_back(emission);
            origins.push_back(origins.back());
            m_GroupCount = static_cast<int>(emissions.size());
            changed = true;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(emissions.empty());
        if (ImGui::Button("Clear Emissions"))
        { emissions.clear(); origins.resize(1u); motion.count = 1u; m_GroupCount = 1; motion.intervalMs = 0u; motion.spreadDegrees = 0.f; changed = true; }
        ImGui::EndDisabled();
        ImGui::SeparatorText("Group Layout");
        ImGui::DragInt("Group count", &m_GroupCount, 1.f, 1, 128, "%d", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Resize Group"))
        {
            if (emissions.empty())
            {
                emissions.resize(motion.count);
                for (uint32_t index = 0; index < motion.count; ++index) emissions[index].startDelayMs = index * motion.intervalMs;
            }
            const auto last = emissions.back();
            emissions.resize(static_cast<size_t>(m_GroupCount), last);
            origins.resize(emissions.size(), origins.back());
            changed = true;
        }
        ImGui::DragFloat3("Step offset (m)", &m_EmissionStep.x, .05f, -100000.f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        EditUInt("Step delay (ms)", m_EmissionDelayStepMs, static_cast<int>(maxDelay));
        ImGui::BeginDisabled(emissions.empty());
        if (ImGui::Button("Arrange Line"))
        {
            auto staged = emissions;
            const auto first = staged.front();
            bool valid = true;
            for (size_t index = 0; index < staged.size(); ++index)
            {
                auto& row = staged[index];
                row.positionOffset = {first.positionOffset.x + m_EmissionStep.x * index,
                    first.positionOffset.y + m_EmissionStep.y * index, first.positionOffset.z + m_EmissionStep.z * index};
                const uint64_t delay = uint64_t(first.startDelayMs) + uint64_t(m_EmissionDelayStepMs) * index;
                valid = valid && delay <= maxDelay && std::isfinite(row.positionOffset.x) && std::isfinite(row.positionOffset.y) &&
                    std::isfinite(row.positionOffset.z) && std::abs(row.positionOffset.x) <= 100000.f &&
                    std::abs(row.positionOffset.y) <= 100000.f && std::abs(row.positionOffset.z) <= 100000.f;
                row.startDelayMs = static_cast<uint32_t>(delay);
            }
            if (valid) { emissions = std::move(staged); changed = true; }
            else m_Status = "Line layout exceeds the Motion lifetime or position limits. Rows preserved.";
        }
        ImGui::DragFloat3("Spacing multiplier", &m_SpacingMultiplier.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Scale Group Spacing"))
        {
            auto staged = emissions;
            bool valid = true;
            for (auto& row : staged)
            {
                row.positionOffset.x *= m_SpacingMultiplier.x;
                row.positionOffset.y *= m_SpacingMultiplier.y;
                row.positionOffset.z *= m_SpacingMultiplier.z;
                valid = valid && std::isfinite(row.positionOffset.x) && std::isfinite(row.positionOffset.y) &&
                    std::isfinite(row.positionOffset.z) && std::abs(row.positionOffset.x) <= 100000.f &&
                    std::abs(row.positionOffset.y) <= 100000.f && std::abs(row.positionOffset.z) <= 100000.f;
            }
            const float3_t orbit{motion.revolutionOffset.x * m_SpacingMultiplier.x,
                motion.revolutionOffset.y * m_SpacingMultiplier.y, motion.revolutionOffset.z * m_SpacingMultiplier.z};
            valid = valid && std::isfinite(orbit.x) && std::isfinite(orbit.y) && std::isfinite(orbit.z) &&
                std::abs(orbit.x) <= 100000.f && std::abs(orbit.y) <= 100000.f && std::abs(orbit.z) <= 100000.f;
            if (valid) { emissions = std::move(staged); motion.revolutionOffset = orbit; changed = true; }
            else m_Status = "Spacing exceeds the Motion limits. Rows preserved.";
        }
        ImGui::EndDisabled();
        ImGui::TextWrapped("Count and delay edits keep linked Collider/Logic rows together when you Save. All boxes using this Motion are updated; unsaved Composition edits are preserved.");
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragInt("Ring Count", &m_RingCount, 1.f, 1, 128, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.f);
        ImGui::DragFloat("Ring Start (deg)", &m_RingStartDegrees, .5f, -360.f, 360.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();
        if (ImGui::Button("Distribute on Ring"))
        {
            /* A row's yaw also turns Revolution Offset, so a row at phase p rides
               R(p)*o and an offset of R(p)*o puts every row on one circle centred
               on the saved position itself. */
            const vector_t orbitOffset = XMLoadFloat3(&motion.revolutionOffset);
            origins.resize(static_cast<size_t>(m_RingCount), origins.empty() ? 0u : origins.back());
            m_GroupCount = m_RingCount;
            emissions.clear();
            for (int index = 0; index < m_RingCount; ++index)
            {
                WORLD_SEQUENCE_OBJECT_EMISSION emission;
                emission.yawDegrees = m_RingStartDegrees + 360.f * static_cast<float>(index) / static_cast<float>(m_RingCount);
                XMStoreFloat3(&emission.positionOffset, XMVector3TransformNormal(orbitOffset,
                    XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees))));
                emissions.push_back(emission);
            }
            changed = true;
        }
        ImGui::TextDisabled("Ring rows circle the saved position at this Motion's Revolution Offset radius; use Revolution (deg/s) for the orbit speed.");
        if (!emissions.empty())
        {
            motion.count = static_cast<uint32_t>(emissions.size());
            motion.intervalMs = 0u;
            motion.spreadDegrees = 0.f;
        }
    }
    if (changed) Mark_Dirty();
    if (!alias) Render_EffectRows(*sequence);
    ImGui::SeparatorText("Selected Key");
    Render_KeyEditor(*sequence);
}

void CWorldObjectTool::Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    if (EditText("Combined Motion Name", resource.displayName)) Mark_Dirty();
    const auto* current = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = current ? m_Document.Find_Template(current->templateId) : nullptr;
    if (ImGui::BeginCombo("Editing Motion", sequence ? sequence->displayName.c_str() : "Choose a row"))
    {
        for (const auto& id : resource.motionInstanceIds)
        {
            const auto* member = m_Document.Find_Instance(id);
            const auto* motion = member ? m_Document.Find_Template(member->templateId) : nullptr;
            ImGui::PushID(id.c_str());
            if (ImGui::Selectable(motion ? motion->displayName.c_str() : id.c_str(), id == m_SelectedInstance)) Select_State(id);
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::TextWrapped("All %zu motions stay in preview. The full editor below changes the selected row; Save keeps all rows.", resource.motionInstanceIds.size());
}

void CWorldObjectTool::Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource)
{
    ImGui::SeparatorText(resource.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    ImGui::SameLine();
    if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter)) m_PreviewDirty = m_PreviewActive;
    ImGui::TextWrapped("Click a motion row or key to edit it in Object Detail. All %zu motions keep playing together.", resource.motionInstanceIds.size());
    if (!m_PreviewStatus.empty()) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    // Disabled rows remain on the authoring timeline, even when playback has a shorter span.
    float extent = 1.f;
    for (const auto& id : resource.motionInstanceIds)
    {
        const auto* instance = m_Document.Find_Instance(id);
        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
        if (sequence) extent = (std::max)(extent, instance->startDelayMs +
            sequence->PresentationSpanMs() / (std::max)(.05f, instance->playbackSpeed));
    }
    float clock = m_ClockMs;
    if (ImGui::SliderFloat("Motion + Effect (ms)", &clock, 0.f, extent, "%.0f")) Seek(clock);
    ImGui::TextDisabled("Playback elapsed: %.0f ms", m_ClockMs);
    ImGui::SetNextItemWidth(180.f); ImGui::SliderFloat("Timeline Zoom", &m_Zoom, 10.f, 500.f, "%.0f px/s");
    if (ImGui::BeginChild("CombinedObjectTimeline", ImVec2(0.f, (std::max)(110.f, ImGui::GetContentRegionAvail().y)),
        true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        auto* draw = ImGui::GetWindowDrawList();
        const uint32_t duration = static_cast<uint32_t>(std::ceil(extent));
        const float width = (std::max)(ImGui::GetContentRegionAvail().x - 12.f, extent * m_Zoom * .001f);
        const float pixelsPerMs = width / extent;
        CompositionTimeline::DrawRuler(draw, origin, ImVec2(origin.x + width, origin.y + 25.f), duration, pixelsPerMs * 1000.f);
        ImGui::InvisibleButton("RulerSeek", ImVec2(width, 25.f));
        if (ImGui::IsItemActive())
            Seek((std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, extent));
        float y = origin.y + 28.f;
        for (const auto& id : resource.motionInstanceIds)
        {
            auto* instance = m_Document.Find_Instance(id);
            auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (!sequence) continue;
            ImGui::PushID(id.c_str());
            const float speed = (std::max)(.05f, instance->playbackSpeed);
            const auto timeX = [&](float localMs) { return origin.x + (instance->startDelayMs + localMs / speed) * pixelsPerMs; };
            ImGui::SetCursorScreenPos(ImVec2(origin.x, y));
            if (ImGui::Selectable(sequence->displayName.c_str(), id == m_SelectedInstance, 0, ImVec2(width, 22.f)))
                Select_State(id);
            y += 24.f;
            for (size_t trackIndex = 0; trackIndex < sequence->tracks.size(); ++trackIndex)
            {
                auto& track = sequence->tracks[trackIndex];
                ImGui::PushID(track.slotId.c_str());
                const ImVec2 row(timeX(0.f), y);
                const ImVec2 end(timeX(static_cast<float>(sequence->durationMs)), y + 25.f);
                CompositionTimeline::DrawBox(draw, row, end,
                    instance->enabled ? IM_COL32(61, 107, 141, 255) : IM_COL32(65, 65, 65, 255),
                    id == m_SelectedInstance && m_SelectedTrack == trackIndex, track.slotId.c_str(), false, false);
                if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row, end) && ImGui::IsMouseClicked(0))
                {
                    if (m_SelectedInstance != id) Select_State(id);
                    m_SelectedTrack = trackIndex; m_SelectedKey = 0;
                }
                for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
                {
                    auto& key = track.keys[keyIndex];
                    const float x = timeX(static_cast<float>(key.timeMs)), centreY = y + 12.f;
                    const bool selected = id == m_SelectedInstance && m_SelectedTrack == trackIndex && m_SelectedKey == keyIndex;
                    draw->AddQuadFilled(ImVec2(x, centreY - 6.f), ImVec2(x + 6.f, centreY),
                        ImVec2(x, centreY + 6.f), ImVec2(x - 6.f, centreY), selected ? IM_COL32(255, 223, 87, 255) : IM_COL32_WHITE);
                    ImGui::PushID(static_cast<int>(keyIndex));
                    ImGui::SetCursorScreenPos(ImVec2((std::clamp)(x - 7.f, origin.x, origin.x + width - 14.f), y + 4.f));
                    ImGui::InvisibleButton("Key", ImVec2(14.f, 18.f));
                    if (ImGui::IsItemClicked())
                    {
                        if (m_SelectedInstance != id) Select_State(id);
                        m_SelectedTrack = trackIndex; m_SelectedKey = keyIndex;
                    }
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && keyIndex > 0 && keyIndex + 1 < track.keys.size())
                    {
                        const float local = ((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs - instance->startDelayMs) * speed;
                        const uint32_t time = static_cast<uint32_t>((std::clamp)(local,
                            static_cast<float>(track.keys[keyIndex - 1].timeMs + 1), static_cast<float>(track.keys[keyIndex + 1].timeMs - 1)));
                        if (time != key.timeMs) { key.timeMs = time; Mark_Dirty(); }
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s / %u ms", track.slotId.c_str(), key.timeMs);
                    ImGui::PopID();
                }
                ImGui::PopID();
                y += 32.f;
            }
            for (const auto& clip : sequence->animationTracks)
            {
                uint32_t end = sequence->durationMs;
                for (const auto& next : sequence->animationTracks)
                    if (next.slotId == clip.slotId && next.startMs > clip.startMs) end = (std::min)(end, next.startMs);
                const ImVec2 first(timeX(static_cast<float>(clip.startMs)), y);
                const ImVec2 last(timeX(static_cast<float>(end)), y + 25.f);
                CompositionTimeline::DrawBox(draw, first, last, IM_COL32(113, 82, 147, 255), id == m_SelectedInstance,
                    clip.displayName.empty() ? clip.clipName.c_str() : clip.displayName.c_str());
                if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(first, last) && ImGui::IsMouseClicked(0)) Select_State(id);
                y += 32.f;
            }
            for (size_t index = 0; index < sequence->effectTracks.size(); ++index)
            {
                const auto& effect = sequence->effectTracks[index];
                const float start = static_cast<float>(sequence->EffectStartMs(effect));
                const ImVec2 first(timeX(start), y), last(timeX(start + effect.durationMs), y + 25.f);
                CompositionTimeline::DrawBox(draw, first, last, IM_COL32(167, 95, 51, 255),
                    id == m_SelectedInstance && m_SelectedEffectRow == index, effect.resourceId.c_str());
                if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(first, last) && ImGui::IsMouseClicked(0))
                { if (m_SelectedInstance != id) Select_State(id); m_SelectedEffectRow = index; }
                y += 32.f;
            }
            ImGui::PopID();
            y += 8.f;
        }
        const float cursor = origin.x + (std::clamp)(m_ClockMs, 0.f, extent) * pixelsPerMs;
        draw->AddLine(ImVec2(cursor, origin.y), ImVec2(cursor, y), IM_COL32(255, 217, 68, 255), 2.f);
        ImGui::SetCursorScreenPos(origin); ImGui::Dummy(ImVec2(width, y - origin.y));
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    ImGui::SeparatorText(sequence.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= PreviewSpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine(); if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* selectedInstance = m_Document.Find_Instance(m_SelectedInstance);
    if (selectedInstance && selectedInstance->anchorKind == "WORLD")
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter))
        {
            m_PreviewDirty = m_PreviewActive;
            if (m_PreviewActive) Seek(m_ClockMs);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Preview at the current character without changing the saved Map position. Clear to preview the authored position.");
    }
    if (selectedInstance) ImGui::TextDisabled("On Complete: %s", MotionEndLabel(selectedInstance->motionEnd));
    if (m_PreviewActive) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    if (m_ClockMs >= SpanMs() && selectedInstance &&
        (selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP || selectedInstance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT))
        ImGui::TextWrapped("Completion motion active. This timeline still edits the selected Motion's first Lifetime; seek to 0 to restart it.");
    float clock = (std::min)(m_ClockMs, SpanMs());
    if (ImGui::SliderFloat("Motion + Effect (ms)", &clock, 0.f, (std::max)(1.f, SpanMs()), "%.0f")) Seek(clock);
    ImGui::TextDisabled("Playback elapsed: %.0f ms", m_ClockMs);
    ImGui::SetNextItemWidth(180.f); ImGui::SliderFloat("Timeline Zoom", &m_Zoom, 10.f, 500.f, "%.0f px/s");
    const float rowHeight = 32.f;
    const uint32_t timelineDuration = sequence.PresentationSpanMs();
    const float width = (std::max)(ImGui::GetContentRegionAvail().x - 12.f, timelineDuration * m_Zoom * .001f);
    const float pixelsPerMs = width / timelineDuration;
    const bool showPhysics = resource && resource->sequenceInstanceId.empty();
    const float tracksHeight = rowHeight * static_cast<float>(sequence.tracks.size() + sequence.animationTracks.size() + sequence.effectTracks.size());
    const float height = 28.f + tracksHeight + (showPhysics ? 64.f : 0.f);
    if (ImGui::BeginChild("ObjectTimeline", ImVec2(0, (std::max)(110.f, ImGui::GetContentRegionAvail().y)), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, origin, ImVec2(origin.x + width, origin.y + 25.f), timelineDuration, pixelsPerMs * 1000.f);
        ImGui::InvisibleButton("RulerSeek", ImVec2(width, 25.f));
        if (ImGui::IsItemActive())
        {
            const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
            if (instance) Seek(instance->startDelayMs + local / instance->playbackSpeed);
        }
        for (size_t index = 0; index < sequence.tracks.size(); ++index)
        {
            auto& track = sequence.tracks[index]; ImGui::PushID(track.slotId.c_str());
            const auto row = ImVec2(origin.x, origin.y + 28.f + rowHeight * index);
            CompositionTimeline::DrawBox(draw, row, ImVec2(row.x + sequence.durationMs * pixelsPerMs, row.y + 25.f),
                IM_COL32(61, 107, 141, 255), m_SelectedTrack == index, track.slotId.c_str(), false, false);
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row, ImVec2(row.x + width, row.y + 25.f)) && ImGui::IsMouseClicked(0))
            { m_SelectedTrack = index; m_SelectedKey = 0; }
            for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
            {
                auto& key = track.keys[keyIndex]; const float x = row.x + key.timeMs * pixelsPerMs;
                const float y = row.y + 12.f;
                const ImU32 color = m_SelectedTrack == index && m_SelectedKey == keyIndex ? IM_COL32(255, 223, 87, 255) : IM_COL32_WHITE;
                draw->AddQuadFilled(ImVec2(x, y - 6), ImVec2(x + 6, y), ImVec2(x, y + 6), ImVec2(x - 6, y), color);
                ImGui::PushID(static_cast<int>(keyIndex)); ImGui::SetCursorScreenPos(ImVec2((std::clamp)(x - 7.f, row.x, row.x + width - 14.f), row.y + 4.f));
                ImGui::InvisibleButton("Key", ImVec2(14, 18));
                if (ImGui::IsItemClicked()) { m_SelectedTrack = index; m_SelectedKey = keyIndex; }
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && keyIndex > 0 && keyIndex + 1 < track.keys.size())
                {
                    const auto moved = static_cast<int>((ImGui::GetIO().MousePos.x - row.x) / pixelsPerMs);
                    const uint32_t time = static_cast<uint32_t>((std::clamp)(moved, static_cast<int>(track.keys[keyIndex - 1].timeMs + 1), static_cast<int>(track.keys[keyIndex + 1].timeMs - 1)));
                    if (time != key.timeMs) { key.timeMs = time; Mark_Dirty(); }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s / %u ms", track.slotId.c_str(), key.timeMs);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            const auto& track = sequence.animationTracks[index];
            uint32_t end = sequence.durationMs;
            for (const auto& next : sequence.animationTracks) if (next.slotId == track.slotId && next.startMs > track.startMs) end = (std::min)(end, next.startMs);
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + index);
            CompositionTimeline::DrawBox(draw, ImVec2(origin.x + track.startMs * pixelsPerMs, y),
                ImVec2(origin.x + end * pixelsPerMs, y + 25.f), IM_COL32(113, 82, 147, 255), false,
                track.displayName.empty() ? track.clipName.c_str() : track.displayName.c_str());
        }
        for (size_t index = 0; index < sequence.effectTracks.size(); ++index)
        {
            const auto& effect = sequence.effectTracks[index];
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + sequence.animationTracks.size() + index);
            const float x = origin.x + sequence.EffectStartMs(effect) * pixelsPerMs;
            const float endX = x + effect.durationMs * pixelsPerMs;
            CompositionTimeline::DrawBox(draw, ImVec2(x, y), ImVec2(endX, y + 25.f),
                IM_COL32(167, 95, 51, 255), m_SelectedEffectRow == index, effect.resourceId.c_str());
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(ImVec2(x, y), ImVec2(endX, y + 25.f)) && ImGui::IsMouseClicked(0))
                m_SelectedEffectRow = index;
        }
        if (showPhysics)
        {
            const float top = origin.y + 28.f + tracksHeight;
            draw->AddRectFilled(ImVec2(origin.x, top), ImVec2(origin.x + width, top + 60.f), IM_COL32(28, 49, 48, 255));
            draw->AddText(ImVec2(origin.x + 5.f, top + 3.f), IM_COL32(136, 227, 198, 255), "Physics Y offset / first emission");
            const auto& motion = sequence.objectMotion;
            const float seconds = sequence.durationMs * .001f;
            const float physicsWidth = sequence.durationMs * pixelsPerMs;
            const auto sampleY = [&motion](float time) { return motion.velocity.y * time + .5f * motion.acceleration.y * time * time; };
            float minimum = (std::min)(0.f, sampleY(seconds));
            float maximum = (std::max)(0.f, sampleY(seconds));
            if (motion.acceleration.y != 0.f)
            {
                const float apex = -motion.velocity.y / motion.acceleration.y;
                if (apex > 0.f && apex < seconds)
                { minimum = (std::min)(minimum, sampleY(apex)); maximum = (std::max)(maximum, sampleY(apex)); }
            }
            const float range = (std::max)(.01f, maximum - minimum);
            if (std::isfinite(motion.velocity.y) && std::isfinite(motion.acceleration.y) &&
                std::isfinite(minimum) && std::isfinite(maximum) && std::isfinite(range))
                for (int segment = 0; segment < 64; ++segment)
                {
                    const float from = static_cast<float>(segment) / 64.f, to = static_cast<float>(segment + 1) / 64.f;
                    draw->AddLine(ImVec2(origin.x + physicsWidth * from, top + 56.f - 30.f * (sampleY(seconds * from) - minimum) / range),
                        ImVec2(origin.x + physicsWidth * to, top + 56.f - 30.f * (sampleY(seconds * to) - minimum) / range), IM_COL32(91, 217, 171, 255), 2.f);
                }
            ImGui::SetCursorScreenPos(ImVec2(origin.x, top));
            ImGui::InvisibleButton("PhysicsSeek", ImVec2(width, 60.f));
            if (ImGui::IsItemActive() && selectedInstance)
            {
                const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(timelineDuration));
                Seek(selectedInstance->startDelayMs + local / selectedInstance->playbackSpeed);
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Drag to seek. Y offset range: %.3f to %.3f m. Added to Transform keys before spread, revolution and anchor rotation.", minimum, maximum);
        }
        const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
        const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
        const float cursorX = origin.x + (std::clamp)(local, 0.f, static_cast<float>(timelineDuration)) * pixelsPerMs;
        draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, origin.y + height), IM_COL32(255, 217, 68, 255), 2.f);
        ImGui::SetCursorScreenPos(origin); ImGui::Dummy(ImVec2(width, height));
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!sequence.tracks.empty())
    {
        m_SelectedTrack = (std::min)(m_SelectedTrack, sequence.tracks.size() - 1);
        if (ImGui::BeginCombo("Target Track", sequence.tracks[m_SelectedTrack].slotId.c_str()))
        {
            for (size_t index = 0; index < sequence.tracks.size(); ++index)
                if (ImGui::Selectable(sequence.tracks[index].slotId.c_str(), index == m_SelectedTrack)) { m_SelectedTrack = index; m_SelectedKey = 0; }
            ImGui::EndCombo();
        }
        auto& track = sequence.tracks[m_SelectedTrack];
        if (!track.keys.empty())
        {
            m_SelectedKey = (std::min)(m_SelectedKey, track.keys.size() - 1);
            auto& key = track.keys[m_SelectedKey];
            if (ImGui::BeginCombo("Keyframe", (std::to_string(key.timeMs) + " ms").c_str()))
            {
                for (size_t index = 0; index < track.keys.size(); ++index)
                    if (ImGui::Selectable((std::to_string(track.keys[index].timeMs) + " ms").c_str(), m_SelectedKey == index)) m_SelectedKey = index;
                ImGui::EndCombo();
            }
            auto& selected = track.keys[m_SelectedKey];
            const bool endpoint = m_SelectedKey == 0 || m_SelectedKey + 1 == track.keys.size();
            bool changed = false;
            ImGui::BeginDisabled(endpoint);
            if (!endpoint) changed |= EditUInt("Key Time (ms)", selected.timeMs, track.keys[m_SelectedKey + 1].timeMs - 1, track.keys[m_SelectedKey - 1].timeMs + 1);
            else ImGui::Text("Endpoint: %u ms", selected.timeMs);
            ImGui::EndDisabled();
            changed |= ImGui::DragFloat3("Position Offset (m)", &selected.positionOffset.x, .01f);
            auto degrees = QuaternionEuler(selected.rotationQuaternion);
            if (ImGui::DragFloat3("Rotation Offset (deg)", &degrees.x, .25f))
            {
                XMStoreFloat4(&selected.rotationQuaternion, XMQuaternionRotationRollPitchYaw(
                    XMConvertToRadians(degrees.x), XMConvertToRadians(degrees.y), XMConvertToRadians(degrees.z)));
                changed = true;
            }
            changed |= ImGui::DragFloat3("Scale Multiplier", &selected.scaleMultiplier.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Visible", &selected.visible);
            if (changed) Mark_Dirty();
            ImGui::BeginDisabled(endpoint);
            if (ImGui::Button("Delete Key")) { track.keys.erase(track.keys.begin() + m_SelectedKey); m_SelectedKey = 0; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::SameLine();
            ImGui::BeginDisabled(track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT || sequence.durationMs < 2);
            if (ImGui::Button("Add Key at Cursor"))
            {
                const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
                const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
                const uint32_t time = static_cast<uint32_t>((std::clamp)(local, 1.f, static_cast<float>(sequence.durationMs - 1)));
                auto at = std::lower_bound(track.keys.begin(), track.keys.end(), time, [](const auto& value, uint32_t clock) { return value.timeMs < clock; });
                m_SelectedKey = at - track.keys.begin();
                if (at == track.keys.end() || at->timeMs != time)
                {
                    auto added = CWorldSequencePlayer::Sample_Track(sequence, track, static_cast<float>(time)); added.timeMs = time;
                    track.keys.insert(at, added); Mark_Dirty();
                }
            }
            ImGui::EndDisabled();
        }
    }
    if (ImGui::CollapsingHeader("Animation Clips"))
    {
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            auto& clip = sequence.animationTracks[index]; ImGui::PushID(static_cast<int>(index));
            ImGui::Text("Slot: %s", clip.slotId.c_str());
            bool changed = EditText("Clip display name", clip.displayName);
            ImGui::TextWrapped("Native clip: %s", clip.clipName.c_str());
            uint32_t minimum = 0, maximum = sequence.durationMs - 1;
            bool first = true;
            for (size_t other = 0; other < sequence.animationTracks.size(); ++other)
            {
                const auto& next = sequence.animationTracks[other];
                if (other == index || next.slotId != clip.slotId) continue;
                if (next.startMs < clip.startMs) { minimum = (std::max)(minimum, next.startMs + 1); first = false; }
                if (next.startMs > clip.startMs) maximum = (std::min)(maximum, next.startMs - 1);
            }
            ImGui::BeginDisabled(first); changed |= EditUInt("Clip Start (ms)", clip.startMs, maximum, minimum); ImGui::EndDisabled();
            changed |= ImGui::DragFloat("Clip Speed", &clip.playbackRate, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Clip Loop", &clip.loop); changed |= ImGui::Checkbox("Hold Last Pose", &clip.holdLastFrame);
            if (changed) Mark_Dirty();
            if (ImGui::SmallButton("Remove Clip"))
            {
                const auto slot = clip.slotId; sequence.animationTracks.erase(sequence.animationTracks.begin() + index);
                for (auto& remaining : sequence.animationTracks) if (remaining.slotId == slot) { remaining.startMs = 0; break; }
                Mark_Dirty(); ImGui::PopID(); break;
            }
            ImGui::PopID();
        }
        ImGui::TextWrapped("Select a native clip in Object Resources, then Append Clip to this Motion.");
    }
}

void CWorldObjectTool::Rebuild_PhysicalTree()
{
    m_PhysicalTree = {};
    const auto search = Lower(m_PhysicalSearch.data());
    for (size_t index = 0; index < m_PhysicalAssets.size(); ++index)
    {
        const auto& asset = m_PhysicalAssets[index];
        if ((m_PhysicalSlot == 0) != (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)) continue;
        if (!search.empty() && Lower(asset.assetId).find(search) == std::string::npos) continue;
        std::vector<std::string> segments;
        for (const auto& segment : std::filesystem::path(asset.assetId).parent_path()) segments.push_back(segment.string());
        InsertResourceTree(m_PhysicalTree, segments, index);
    }
    FinalizeResourceTree(m_PhysicalTree);
}

void CWorldObjectTool::Render_PhysicalResources()
{
    ImGui::SeparatorText("Physical Resources");
    if (!m_SelectedInstance.empty()) ImGui::TextWrapped("Select the parent Object to change its shared model or texture.");
    if (!m_PhysicalScanned)
    {
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
        m_PhysicalScanned = true;
    }
    ImGui::BeginDisabled(m_PhysicalScanRunning);
    if (ImGui::Button("Refresh Files"))
        m_PhysicalScanRunning = m_PhysicalScan.Begin({"Effect", "Map", "Deploy", "Character"}, m_PhysicalStatus);
    ImGui::EndDisabled();
    if (m_PhysicalScanRunning)
    { ImGui::SameLine(); ImGui::TextDisabled("Scanning files..."); }
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::Combo("##AssignSlot", &m_PhysicalSlot, "Model (.wmodel)\0Diffuse (.dds)\0")) Rebuild_PhysicalTree();
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::InputTextWithHint("##PhysicalSearch", "Search full relative path", m_PhysicalSearch.data(), m_PhysicalSearch.size())) Rebuild_PhysicalTree();
    ImGui::TextDisabled("%zu matching files", m_PhysicalTree.iRecursiveLeafCount);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_PhysicalStatus.c_str());
    if (ImGui::BeginChild("PhysicalResourceFolders", ImVec2(0, (std::max)(100.f, ImGui::GetContentRegionAvail().y * .40f)), true))
    {
        RenderResourceTree(m_PhysicalTree, [this](size_t index) {
            if (index >= m_PhysicalAssets.size()) return;
            const auto& asset = m_PhysicalAssets[index];
            ImGui::PushID(asset.assetId.c_str());
            auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
            ImGui::BeginDisabled(!resource || !m_SelectedInstance.empty() || !resource->sequenceInstanceId.empty());
            if (ImGui::Selectable(asset.fileName.c_str(), asset.assetId == m_SelectedPhysical &&
                (asset.kind != PHYSICAL_RESOURCE_KIND::MODEL || m_AnimationCandidateObjectId == m_SelectedObject)))
            {
                m_SelectedPhysical = asset.assetId;
                if (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)
                {
                    m_AnimationCandidateModelAssetId = asset.assetId;
                    m_AnimationCandidateObjectId = m_SelectedObject;
                    Refresh_AnimationResources();
                    m_Status = "Model candidate selected. Assign Model updates the parent Object; its child motions keep their clip bindings.";
                }
                else
                {
                    resource->diffuseTextureAssetId = asset.assetId;
                    const auto pristinePattern = m_PristinePatternId;
                    Mark_Dirty();
                    m_PristinePatternId = pristinePattern;
                    m_Status = "Assigned diffuse " + asset.assetId + ". Save stores it on " + resource->displayName + ".";
                }
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", asset.assetId.c_str());
            ImGui::PopID();
        });
    }
    ImGui::EndChild();
}
#endif
