#include "imgui.h"
#include "CharacterActionWorkbench.h"
#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "ActorCatalog.h"
#include "Animation.h"
#include "BinaryAsset/WModelDecoder.h"
#include "RuntimeAssetRoot.h"
#include "Animation_Tool.h"
#include "CharacterPreviewPanel.h"
#include "CompositionTimeline.h"
#include "DataJson.h"
#include "EffectAuthoringResourceTree.h"
#include "EffectAuthoringSequencer.h"
#include "EffectEditingSession.h"
#include "GameInstance.h"
#include "Model.h"
#include "PlayerSkillCatalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <set>
#include <string_view>
#include <unordered_map>

namespace Client
{
namespace
{
using CLASS = LostArk::Shared::CHARACTER_CLASS_ID;
struct CLASS_ENTRY { CLASS id; const char* asset; const char* label; };
constexpr std::array<CLASS_ENTRY, 6> CLASSES{{
    {CLASS::LANCE_MASTER, "LanceMaster", "Lance Master"},
    {CLASS::GUNSLINGER, "GunSlinger", "Gunslinger"},
    {CLASS::SLAYER, "Slayer", "Slayer"},
    {CLASS::ARTIST, "Artist", "Artist"},
    {CLASS::DIMENSIONMASTER, "DimensionMaster", "Dimension Master"},
    {CLASS::WARLORD, "Warlord", "Warlord"}
}};
// Same category order as the Boss workbench, minus the boss-only families.
constexpr std::array<const char*, 7> RESOURCE_CATEGORIES{{"Animation", "Logic", "Effect", "Collider", "Sound", "Camera", "Pattern"}};
constexpr std::array<const char*, 7> LANE_LABELS{{"Stage", "Animation", "Logic", "Effect", "Collider", "Sound", "Camera"}};
constexpr std::uint32_t HIT_TICK_MS = 34u;
constexpr std::uint32_t MAX_EDITOR_TIME_MS = 600000u;
constexpr ImVec4 WARNING_TEXT{1.f, .6f, .45f, 1.f};

bool Read_Source(const std::filesystem::path& path, std::string& text)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    text.assign(std::istreambuf_iterator<char>(input), {});
    return !input.bad();
}

int Slot_Order(const std::string& slot)
{
    constexpr std::array<const char*, 17> slots{{"LMB", "SPACE", "Q", "W", "E", "R", "A", "S", "D", "F", "T", "Z", "X", "V", "ALT_V", "ALT_Q", "ALT_W"}};
    const auto found = std::find(slots.begin(), slots.end(), slot);
    return static_cast<int>(found - slots.begin());
}

bool Edit_U32(const char* label, std::uint32_t& value, int maximum = 600000)
{
    int edit = static_cast<int>(value);
    if (!ImGui::DragInt(label, &edit, 1.f, 0, maximum)) return false;
    value = static_cast<std::uint32_t>((std::clamp)(edit, 0, maximum));
    return true;
}

bool Edit_Double(const char* label, double& value, float minimum, float maximum)
{
    float edit = static_cast<float>(value);
    if (!ImGui::DragFloat(label, &edit, .01f, minimum, maximum, "%.3f")) return false;
    if (!std::isfinite(edit)) return false;
    value = (std::clamp)(edit, minimum, maximum);
    return true;
}

bool Read_U32Field(const DATA_JSON_VALUE& owner, const char* name, std::uint32_t& value)
{
    const auto* found = owner.Find(name);
    if (!found || !found->Is_Number() || !std::isfinite(found->Get_Number())) return false;
    const double number = found->Get_Number();
    if (number < 0.0 || number > 4294967295.0 || std::floor(number) != number) return false;
    value = static_cast<std::uint32_t>(number);
    return true;
}

const char* Skill_KindLabel(const LostArk::Shared::PLAYER_SKILL_KIND kind)
{
    switch (kind)
    {
    case LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE: return "ACTIVE";
    case LostArk::Shared::PLAYER_SKILL_KIND::COMBO: return "COMBO";
    case LostArk::Shared::PLAYER_SKILL_KIND::HOLD: return "HOLD";
    case LostArk::Shared::PLAYER_SKILL_KIND::COUNTER: return "COUNTER";
    case LostArk::Shared::PLAYER_SKILL_KIND::STANDUP: return "STANDUP";
    default: return "UNKNOWN";
    }
}

// Overlapping boxes inside one lane get extra display rows. Rows are display
// only; they never create or reorder saved tracks (same rule as the Boss lanes).
struct LANE_INTERVAL final
{
    std::string id;
    std::uint64_t startMs = 0u, endMs = 0u;
};
std::size_t Allocate_TimelineDisplayRows(std::vector<LANE_INTERVAL> intervals,
    std::unordered_map<std::string, std::size_t>& rows)
{
    std::stable_sort(intervals.begin(), intervals.end(), [](const auto& a, const auto& b) { return a.startMs < b.startMs; });
    std::vector<std::uint64_t> rowEnds;
    for (const auto& interval : intervals)
    {
        std::size_t row = 0u;
        while (row < rowEnds.size() && rowEnds[row] > interval.startMs) ++row;
        if (row == rowEnds.size()) rowEnds.push_back(interval.endMs); else rowEnds[row] = interval.endMs;
        rows.emplace(interval.id, row);
    }
    return (std::max)(std::size_t{1u}, rowEnds.size());
}

void Help_Marker(const char* text)
{
    ImGui::SameLine(); ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", text);
}
}

CCharacterActionWorkbench::CCharacterActionWorkbench(std::shared_ptr<CCharacterPreviewPanel> panel,
    std::shared_ptr<CEffectAuthoringSequencer> sequencer, CAnimation_Tool* animationTool)
    : m_Panel(std::move(panel)), m_Sequencer(std::move(sequencer)), m_AnimationTool(animationTool)
{
}
CCharacterActionWorkbench::~CCharacterActionWorkbench()
{
    On_WorkbenchDeactivated();
    if (m_Panel) m_Panel->Set_SessionLock(CHARACTER_PREVIEW_LOCK_OWNER::CHARACTER_ACTION_WORKBENCH, false, {});
}
void CCharacterActionWorkbench::Set_OpenEffectResourceCallback(std::function<void(const std::string&)> callback)
{ m_OpenEffect = std::move(callback); }
void CCharacterActionWorkbench::Set_Camera(const std::shared_ptr<Engine::CCamera>& camera)
{ if (m_Sequencer) m_Sequencer->Set_Camera(camera); }
void CCharacterActionWorkbench::Update(const float dt, const bool active)
{
    if (!m_Sequencer) return;
    m_Sequencer->Update(dt, active);
    if (!m_PreviewDirty) m_PreviewClock = m_Sequencer->ClockMs();
}
bool CCharacterActionWorkbench::Consume_InteractionRequest()
{
    const bool sequencerInteraction = m_Sequencer && m_Sequencer->Consume_InteractionRequest();
    const bool result = m_Interaction || sequencerInteraction;
    m_Interaction = false;
    return result;
}
void CCharacterActionWorkbench::On_WorkbenchDeactivated()
{
    if (m_Sequencer)
    {
        if (!m_PreviewDirty || m_Sequencer->Is_Active()) m_PreviewClock = m_Sequencer->ClockMs();
        m_Sequencer->Stop();
    }
    Stop_SoundPreview();
    m_CueEditorClip.clear();
    m_PreviewDirty = true;
}
void CCharacterActionWorkbench::On_LevelChanged()
{ On_WorkbenchDeactivated(); m_Generation = 0u; m_PreviewDirty = true; }
bool CCharacterActionWorkbench::Target_IsCurrent() const
{
    return m_Panel && m_Panel->Is_PreviewActive() && !m_Asset.empty() &&
        m_Asset == CAnimationTargetService::Resolve_AssetName() &&
        m_Generation == CAnimationTargetService::Resolve_TargetGeneration();
}
bool CCharacterActionWorkbench::Restore_PreviewTarget()
{
    if (Target_IsCurrent()) return true;
    if (!m_Panel || m_ClassIndex < 0 || m_ClassIndex >= static_cast<int>(CLASSES.size()) ||
        m_Asset != CLASSES[m_ClassIndex].asset)
    { m_Status = "Select a Character action before restoring its preview."; return false; }
    // Reconstitute this draft's exact owner after a level change. Other tools
    // retain their locks; Animation Tool separately validates its own cue owner.
    struct OWN_LOCK_RESTORE final
    {
        CCharacterPreviewPanel& panel; bool locked;
        ~OWN_LOCK_RESTORE() { panel.Set_SessionLock(CHARACTER_PREVIEW_LOCK_OWNER::CHARACTER_ACTION_WORKBENCH,
            locked, "Save the Character action draft before changing its preview model."); }
    } restore{*m_Panel, m_BindingsDirty || m_CombatDirty};
    m_Panel->Set_SessionLock(CHARACTER_PREVIEW_LOCK_OWNER::CHARACTER_ACTION_WORKBENCH, false, {});
    const bool selected = m_AnimationTool ? m_AnimationTool->Restore_CharacterActionPreview(m_Asset, m_Status) :
        m_Panel->Select_TargetAsset(m_Asset);
    if (!selected)
    { if (!m_AnimationTool) m_Status = m_Panel->Get_Status(); return false; }
    m_Generation = CAnimationTargetService::Resolve_TargetGeneration();
    m_PreviewDirty = m_RowsDirty = true;
    return Target_IsCurrent();
}
bool CCharacterActionWorkbench::Has_Draft() const
{
    return m_BindingsDirty || m_CombatDirty ||
        (m_AnimationTool && m_AnimationTool->Has_CharacterActionCueChanges(m_Asset));
}
void CCharacterActionWorkbench::Begin_WorkbenchFrame()
{
    if (!m_CatalogLoaded) m_CatalogLoaded = CPlayerSkillCatalog::Load(m_Status);
    // One read per session; Reload re-reads it. A failed read leaves timing rows out with a status.
    if (m_CatalogLoaded && !m_TimingsLoaded) { m_TimingsLoaded = true; Load_SkillTimings(); }
    if (m_Panel) m_Panel->Refresh_Level();
    if (m_RowsDirty && Target_IsCurrent()) Rebuild_Rows();
}
void CCharacterActionWorkbench::End_WorkbenchFrame()
{
    if (m_AddCollider) { m_AddCollider = false; Add_Collider(); }
    if (!m_RemoveCollider.empty()) { const auto retired = m_RemoveCollider; m_RemoveCollider.clear(); Remove_Collider(retired); }
    if (m_SaveBindings) { m_SaveBindings = false; Save_Bindings(); }
    if (m_SaveCombat) { m_SaveCombat = false; Save_Combat(); }
    if (m_Reload) { m_Reload = false; Reload_Selected(); }
    if (m_Panel) m_Panel->Set_SessionLock(CHARACTER_PREVIEW_LOCK_OWNER::CHARACTER_ACTION_WORKBENCH,
        m_BindingsDirty || m_CombatDirty, "Save the Character action draft before changing its preview model.");
}
void CCharacterActionWorkbench::Render_WorkbenchPane(const COMPOSITION_WORKBENCH_PANE pane)
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) m_Interaction = true;
    switch (pane)
    {
    case COMPOSITION_WORKBENCH_PANE::PATTERNS: Render_Actions(); break;
    case COMPOSITION_WORKBENCH_PANE::SEQUENCER: Render_Timeline(); break;
    case COMPOSITION_WORKBENCH_PANE::RESOURCES: Render_Resources(); break;
    case COMPOSITION_WORKBENCH_PANE::DETAILS: Render_Details(); break;
    case COMPOSITION_WORKBENCH_PANE::PREVIEW:
        ImGui::Text("Character: %s", m_Asset.empty() ? "Select an action" : m_Asset.c_str());
        ImGui::TextWrapped("The preview uses the complete character, equipment and weapon on the shared authoring target.");
        Render_Transport();
        ImGui::TextWrapped("%s", m_Status.c_str());
        if (!Target_IsCurrent() && !m_Asset.empty() && ImGui::Button("Restore selected preview"))
            Restore_PreviewTarget();
        break;
    case COMPOSITION_WORKBENCH_PANE::TOOLBAR: Render_Transport(); break;
    default: break;
    }
}

void CCharacterActionWorkbench::Render_Actions()
{
    ImGui::SeparatorText("Character Actions");
    if (!m_CatalogLoaded) { ImGui::TextWrapped("%s", m_Status.c_str()); return; }
    ImGui::InputTextWithHint("##CharacterActionSearch", "Slot, skill ID or name", m_Search, sizeof(m_Search));
    for (std::size_t i = 0u; i < CLASSES.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::TreeNodeEx(CLASSES[i].label, ImGuiTreeNodeFlags_DefaultOpen))
        {
            std::vector<const PLAYER_SKILL_DEFINITION*> skills;
            for (const auto& skill : CPlayerSkillCatalog::Get_Skills())
                if (skill.eCharacterClass == CLASSES[i].id) skills.push_back(&skill);
            std::stable_sort(skills.begin(), skills.end(), [](const auto* a, const auto* b) {
                return Slot_Order(a->strInputSlot) < Slot_Order(b->strInputSlot); });
            for (const auto* skill : skills)
            {
                const std::string label = skill->strInputSlot + " | " + skill->strDisplayName + " | " + std::to_string(skill->iSkillId);
                if (m_Search[0] && label.find(m_Search) == std::string::npos) continue;
                ImGui::PushID(static_cast<int>(skill->iSkillId));
                const bool selected = m_ClassIndex == static_cast<int>(i) && m_SkillId == skill->iSkillId;
                const bool staged = skill->iComboStageCount > 1u;
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (selected && !m_Stage) flags |= ImGuiTreeNodeFlags_Selected;
                if (!staged) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                const bool open = ImGui::TreeNodeEx("Action", flags, "%s", label.c_str());
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) Select_Action(static_cast<int>(i), skill->iSkillId, {});
                if (staged && open)
                {
                    for (std::uint32_t stage = 0u; stage < skill->iComboStageCount; ++stage)
                    {
                        const std::string stageLabel = "Stage " + std::to_string(stage + 1u);
                        if (ImGui::Selectable(stageLabel.c_str(), selected && m_Stage == stage)) Select_Action(static_cast<int>(i), skill->iSkillId, stage);
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::TextWrapped("%s", m_Status.c_str());
}

ANIMATION_SKILL_BINDING* CCharacterActionWorkbench::Selected_Binding()
{
    const auto found = std::find_if(m_Bindings.Bindings.begin(), m_Bindings.Bindings.end(),
        [&](const auto& row) { return row.iSkillId == m_SkillId; });
    return found == m_Bindings.Bindings.end() ? nullptr : &*found;
}
const ANIMATION_SKILL_BINDING* CCharacterActionWorkbench::Selected_Binding() const
{ return const_cast<CCharacterActionWorkbench*>(this)->Selected_Binding(); }
ANIMATION_SKILL_CLIP* CCharacterActionWorkbench::Find_Clip(const std::string& id)
{
    if (auto* binding = Selected_Binding())
        for (auto& stage : binding->Stages)
            for (auto& clip : stage.Clips)
                if (clip.strClipOccurrenceId == id) return &clip;
    return nullptr;
}
bool CCharacterActionWorkbench::Load_SkillTimings()
{
    // The same balance document the combat owner re-reads at Save. It stays
    // Server truth: this Workbench only mirrors action, hit and combo windows.
    std::string bytes, status;
    DATA_JSON_VALUE root;
    if (!Read_Source(CProjectDataRoot::Resolve("Balance/PlayerSkills.json"), bytes) || !CDataJson::Parse(bytes, root, status))
    { m_Status = "Cannot read PlayerSkills.json for Server timing: " + status; return false; }
    const auto* skills = root.Find("skills");
    if (!skills || !skills->Is_Array()) { m_Status = "PlayerSkills.json has no skills array; Server timing rows are unavailable."; return false; }
    std::unordered_map<std::uint32_t, std::vector<STAGE_TIMING>> timings;
    for (const auto& skill : skills->Get_Array())
    {
        std::uint32_t skillId = 0u;
        STAGE_TIMING base;
        if (!Read_U32Field(skill, "skillId", skillId) || !Read_U32Field(skill, "actionDurationMs", base.actionDurationMs) ||
            !Read_U32Field(skill, "hitTimeMs", base.hitTimeMs)) continue; // The catalog loader already reports malformed rows.
        const auto* kind = skill.Find("skillKind");
        base.combo = kind && kind->Is_String() && kind->Get_String() == "COMBO";
        std::vector<STAGE_TIMING> stages;
        if (const auto* combo = skill.Find("comboStages"); combo && combo->Is_Array())
            for (const auto& stage : combo->Get_Array())
            {
                STAGE_TIMING timing = base;
                if (!Read_U32Field(stage, "actionDurationMs", timing.actionDurationMs) || !Read_U32Field(stage, "hitTimeMs", timing.hitTimeMs) ||
                    !Read_U32Field(stage, "comboAdvanceMs", timing.comboAdvanceMs) || !Read_U32Field(stage, "inputOpenMs", timing.inputOpenMs) ||
                    !Read_U32Field(stage, "inputCloseMs", timing.inputCloseMs))
                { stages.clear(); break; }
                stages.push_back(timing);
            }
        if (stages.empty()) stages.push_back(base);
        timings[skillId] = std::move(stages);
    }
    m_SkillTimings = std::move(timings);
    return true;
}
const CCharacterActionWorkbench::STAGE_TIMING* CCharacterActionWorkbench::Stage_Timing(const std::uint32_t stage) const
{
    const auto found = m_SkillTimings.find(m_SkillId);
    if (found == m_SkillTimings.end() || stage >= found->second.size()) return nullptr;
    return &found->second[stage];
}
std::uint32_t CCharacterActionWorkbench::Collider_LimitMs(const std::uint32_t stage) const
{
    // Save_Atomic rejects caster hits past actionDurationMs, and past comboAdvanceMs on COMBO stages.
    const auto* timing = Stage_Timing(stage);
    if (!timing) return MAX_EDITOR_TIME_MS;
    return timing->combo ? (std::min)(timing->actionDurationMs, timing->comboAdvanceMs) : timing->actionDurationMs;
}
bool CCharacterActionWorkbench::Load_Class(const int classIndex)
{
    if (classIndex < 0 || classIndex >= static_cast<int>(CLASSES.size()) || !m_Panel) return false;
    const auto& entry = CLASSES[classIndex];
    if (Has_Draft()) { m_Status = "Save the existing action and cue drafts before changing or reloading their owner."; return false; }
    ANIMATION_SKILL_BINDING_DOCUMENT bindings;
    std::string baseline;
    if (!Read_Source(CAnimationSkillBindingDocument::Resolve_Path(entry.asset), baseline))
    { m_Status = std::string("Cannot read Character skill binding: ") + entry.asset; return false; }
    if (!CAnimationSkillBindingDocument::Parse_Text(baseline, bindings, m_Status)) return false;
    const auto* actor = CActorCatalog::Find_Character(entry.id);
    if (!actor || actor->runtimeStatus != "supported") { m_Status = "The Character runtime catalog entry is unavailable."; return false; }
    std::vector<std::string> sources{actor->bodyModel};
    sources.insert(sources.end(), actor->animationSetModels.begin(), actor->animationSetModels.end());
    std::vector<Engine::MODEL_ANIMATION_CATALOG_ENTRY> metadata;
    std::vector<std::string> names;
    for (const auto& source : sources)
    {
        std::vector<Engine::MODEL_ANIMATION_CATALOG_ENTRY> sourceClips;
        if (!Engine::CWModelDecoder::Read_AnimationCatalog(CRuntimeAssetRoot::Resolve(source), sourceClips, m_Status)) return false;
        for (const auto& clip : sourceClips) { names.push_back(clip.name); metadata.push_back(clip); }
    }
    if (!CAnimationSkillBindingDocument::Validate(bindings, entry.asset, entry.id, CPlayerSkillCatalog::Get_Skills(), names, m_Status)) return false;
    for (const auto& binding : bindings.Bindings)
        for (const auto& stage : binding.Stages)
            for (const auto& clip : stage.Clips)
            {
                const auto source = std::find_if(metadata.begin(), metadata.end(), [&](const auto& value) { return value.name == clip.strClipName; });
                if (source == metadata.end()) { m_Status = "Missing model clip: " + clip.strClipName; return false; }
                const ACTION_PRESENTATION_CLIP_TIMING timing{source->durationTicks / Engine::CAnimation::COOKED_TICK_RATE,
                    clip.iPlayMs, clip.fPlayRate, false, clip.iSourceStartMs * .001f};
                float sourceDuration = 0.f, wallDuration = 0.f;
                if (!CActionPresentationTimeline::Resolve_ClipDuration(timing, sourceDuration, wallDuration))
                { m_Status = "Invalid model source window: " + clip.strClipName; return false; }
            }
    ANIMATION_EFFECT_CUE_DOCUMENT cues;
    if (!CAnimationEffectCueDocument::Load(entry.asset, names, cues, m_Status)) return false;
    CCharacterActionCombatDocument combat;
    if (!combat.Reload(entry.asset, m_Status)) return false;
    // The sound class snapshot is display data; a missing class row keeps the
    // action usable and the Sound tab reports the catalog status instead.
    CSoundCueCatalog::EVENT_VARIANTS soundEvents;
    std::string soundStatus;
    if (!CSoundCueCatalog::Load_ClassSnapshot(entry.asset, soundEvents, soundStatus)) soundEvents.clear();
    std::vector<std::string> soundNames;
    for (const auto& [eventName, variants] : soundEvents) soundNames.push_back(eventName);
    std::sort(soundNames.begin(), soundNames.end());
    // All file/owner/source-window checks precede replacing the shared preview.
    // CharacterPreviewPanel stages geometry/equipment before committing its target.
    if (!m_Panel->Select_TargetAsset(entry.asset)) { m_Status = m_Panel->Get_Status(); return false; }
    for (auto& binding : bindings.Bindings)
        for (auto& stage : binding.Stages)
            for (auto& clip : stage.Clips)
                if (clip.strClipOccurrenceId.empty()) clip.strClipOccurrenceId = CEffectEditingSession::New_Id("character.clip.");
    On_WorkbenchDeactivated();
    m_Bindings = std::move(bindings); m_Cues = std::move(cues); m_Combat = std::move(combat);
    m_CombatRows = m_Combat.Get_Rows(); m_ClipNames = std::move(names); m_Baseline = std::move(baseline);
    m_SoundEvents = std::move(soundEvents); m_SoundEventNames = std::move(soundNames); m_SoundStatus = std::move(soundStatus);
    m_Asset = entry.asset; m_ClassIndex = classIndex; m_Generation = CAnimationTargetService::Resolve_TargetGeneration();
    m_BindingsDirty = m_CombatDirty = m_CombatPublishPending = false; m_PreviewDirty = true; m_SelectedRow.clear();
    m_Status = "Loaded Product bindings, cues and independent Collider / Logic / Result rows.";
    return true;
}
bool CCharacterActionWorkbench::Select_Action(const int classIndex, const std::uint32_t skillId,
    const std::optional<std::uint32_t> stage)
{
    if (m_ClassIndex != classIndex && !Load_Class(classIndex)) return false;
    if (!Restore_PreviewTarget()) return false;
    const auto* definition = CPlayerSkillCatalog::Find_ById(skillId);
    if (!definition || definition->eCharacterClass != CLASSES[classIndex].id) return false;
    On_WorkbenchDeactivated();
    m_SkillId = skillId; m_Stage = stage; m_SelectedRow.clear(); m_PreviewDirty = true; m_PreviewClock = 0u;
    if (!Selected_Binding()) { m_Status = "The selected skill is missing its Product animation binding."; return false; }
    return Rebuild_Rows();
}
bool CCharacterActionWorkbench::Reload_Selected()
{
    if (Has_Draft()) { m_Status = "Reload preserved the unsaved draft. Save its owner first."; return false; }
    if (m_ClassIndex < 0) return false;
    const auto skillId = m_SkillId; const auto stage = m_Stage;
    (void)Load_SkillTimings();
    return Load_Class(m_ClassIndex) && Select_Action(m_ClassIndex, skillId, stage);
}

bool CCharacterActionWorkbench::Clip_Duration(const ANIMATION_SKILL_CLIP& clip, std::uint32_t& duration) const
{
    const auto model = CAnimationTargetService::Resolve_Model();
    if (!model || !Target_IsCurrent()) return false;
    std::uint32_t found = UINT32_MAX;
    for (std::uint32_t i = 0u; i < model->Get_NumAnimations(); ++i)
        if (const char* name = model->Get_AnimationName(i); name && clip.strClipName == name)
        { if (found != UINT32_MAX) return false; found = i; }
    float position = 0.f, ticks = 0.f;
    if (found == UINT32_MAX || !model->Get_AnimationProgress(found, position, ticks)) return false;
    const float tps = model->Get_AnimationTickPerSecond(found);
    if (!std::isfinite(tps) || tps <= 0.f) return false;
    const ACTION_PRESENTATION_CLIP_TIMING timing{ticks / tps, clip.iPlayMs, clip.fPlayRate, false, clip.iSourceStartMs * .001f};
    float source = 0.f, wall = 0.f;
    if (!CActionPresentationTimeline::Resolve_ClipDuration(timing, source, wall)) return false;
    duration = (std::max)(1u, static_cast<std::uint32_t>(std::ceil(wall * 1000.f)));
    return true;
}

bool CCharacterActionWorkbench::Rebuild_Rows()
{
    const auto* binding = Selected_Binding();
    if (!binding) return false;
    const auto* skill = CPlayerSkillCatalog::Find_ById(m_SkillId);
    std::vector<ROW> rows;
    std::vector<STAGE_SUMMARY> summaries(binding->Stages.size());
    // The clock advances by clip wall time only, exactly like the preview
    // sequencer, so boxes stay aligned with Play/Seek. Server stage locks that
    // outlast the clips are drawn as Stage / Post-delay boxes on that same clock.
    std::uint32_t clock = 0u, end = 1u, canvas = 1u, lastActionEnd = 0u, lastStage = 0u;
    bool timingMissing = false;
    for (std::size_t stage = 0u; stage < binding->Stages.size(); ++stage)
    {
        if (m_Stage && *m_Stage != stage) continue;
        const auto stageId = static_cast<std::uint32_t>(stage);
        const std::uint32_t stageStart = clock;
        for (const auto& clip : binding->Stages[stage].Clips)
        {
            std::uint32_t duration = 0u;
            if (!Clip_Duration(clip, duration)) { m_Status = "Invalid source window or missing clip: " + clip.strClipName; return false; }
            ROW animation{ROW_KIND::ANIMATION, clip.strClipOccurrenceId, "Animation | " + clip.strClipName,
                clip.strClipName, {}, "skillbindings.json", clock, duration, stageId, LANE::ANIMATION};
            animation.editable = true;
            rows.push_back(std::move(animation));
            const std::uint32_t sourceEnd = clip.iSourceStartMs + static_cast<std::uint32_t>(std::ceil(duration * clip.fPlayRate));
            const auto within = [&](std::uint32_t time) { return time >= clip.iSourceStartMs && time < sourceEnd; };
            const auto local = [&](std::uint32_t time) { return static_cast<std::uint32_t>((time - clip.iSourceStartMs) / clip.fPlayRate); };
            for (const auto& cue : m_Cues.Cues)
            {
                if (cue.strClipName != clip.strClipName || !within(cue.iStartMs)) continue;
                const auto length = cue.iEndMs > cue.iStartMs ? static_cast<std::uint32_t>((cue.iEndMs - cue.iStartMs) / clip.fPlayRate) : 100u;
                ROW row{ROW_KIND::EFFECT, clip.strClipOccurrenceId + ".effect." + cue.strEffectAssetId + "." + std::to_string(cue.iStartMs),
                    "Effect | " + cue.strEffectAssetId, clip.strClipName, cue.strEffectAssetId, "Cue owner: .animevents; anchor: " + cue.strAnchorSlotId,
                    clock + local(cue.iStartMs), (std::max)(1u, length), stageId, LANE::EFFECT};
                row.source = cue.iStartMs;
                rows.push_back(std::move(row));
            }
            for (const auto& cue : m_Cues.Sounds)
            {
                if (cue.strClipName != clip.strClipName || !within(cue.iStartMs)) continue;
                ROW row{ROW_KIND::SOUND, clip.strClipOccurrenceId + ".sound." + cue.strEventName + "." + std::to_string(cue.iStartMs),
                    "Sound | " + cue.strEventName, clip.strClipName, cue.strEventName, "Cue owner: .animevents / variants: CharacterSoundCatalog.json",
                    clock + local(cue.iStartMs), 80u, stageId, LANE::SOUND};
                row.source = cue.iStartMs;
                // Dragging writes the same .animevents SOUND row through CAnimation_Tool.
                row.editable = true;
                rows.push_back(std::move(row));
            }
            for (const auto& cue : m_Cues.Shakes)
            {
                if (cue.strClipName != clip.strClipName || !within(cue.iStartMs)) continue;
                const auto shakeMs = static_cast<std::uint32_t>((std::max)(0.f, cue.Spec.fDurationSeconds) * 1000.f / clip.fPlayRate);
                ROW row{ROW_KIND::SHAKE, clip.strClipOccurrenceId + ".shake." + std::to_string(cue.iStartMs),
                    "Camera Shake", clip.strClipName, {}, "Cue owner: .animevents (SHAKE payload)", clock + local(cue.iStartMs),
                    (std::max)(80u, shakeMs), stageId, LANE::CAMERA};
                row.source = cue.iStartMs;
                rows.push_back(std::move(row));
            }
            clock += duration;
        }
        const std::uint32_t clipWall = clock - stageStart;
        const auto* timing = Stage_Timing(stageId);
        if (!timing) timingMissing = true;
        const std::uint32_t actionMs = timing ? timing->actionDurationMs : 0u;
        const std::uint32_t stageDuration = (std::max)(1u, (std::max)(clipWall, actionMs));
        summaries[stage] = {stageStart, clipWall, actionMs, timing != nullptr};
        lastActionEnd = stageStart + stageDuration; lastStage = stageId;
        {
            char label[160]{};
            if (timing) std::snprintf(label, sizeof(label), "Stage %u | clips %u ms | Server %u ms", stageId + 1u, clipWall, actionMs);
            else std::snprintf(label, sizeof(label), "Stage %u | clips %u ms | Server timing unavailable", stageId + 1u, clipWall);
            ROW row{ROW_KIND::STAGE, "character.stage." + std::to_string(stage), label, {}, {},
                "Server stage lock = PlayerSkills.json actionDurationMs (Balance Tool + Publish-GameplayBalance.ps1). Clips are presentation only.",
                stageStart, stageDuration, stageId, LANE::STAGE};
            row.warning = timing && clipWall > actionMs;
            rows.push_back(std::move(row));
        }
        if (timing && actionMs > clipWall)
        {
            ROW row{ROW_KIND::GAP, "character.gap." + std::to_string(stage), "Post-delay | " + std::to_string(actionMs - clipWall) + " ms", {}, {},
                "The character holds its last pose until the Server action lock ends (actionDurationMs - clip wall end).",
                stageStart + clipWall, actionMs - clipWall, stageId, LANE::ANIMATION};
            rows.push_back(std::move(row));
        }
        if (timing)
        {
            const auto timingRow = [&](const char* kind, const std::uint32_t start, const std::uint32_t duration, std::string detail)
            {
                ROW row{ROW_KIND::TIMING, "character.timing." + std::to_string(stage) + "." + kind, std::string("Timing | ") + kind, {}, {},
                    std::move(detail), start, (std::max)(1u, duration), stageId, LANE::LOGIC};
                rows.push_back(std::move(row));
            };
            timingRow("ACTION_LOCK", stageStart, actionMs, "PlayerSkills actionDurationMs: the Server refuses other actions during this window.");
            timingRow("HIT", stageStart + timing->hitTimeMs, HIT_TICK_MS, "PlayerSkills hitTimeMs: the Server hit tick for skills without HitShapes rows.");
            if (timing->combo)
            {
                if (timing->inputCloseMs > timing->inputOpenMs)
                    timingRow("COMBO_INPUT", stageStart + timing->inputOpenMs, timing->inputCloseMs - timing->inputOpenMs,
                        "PlayerSkills inputOpenMs..inputCloseMs: the next combo press is accepted inside this window.");
                timingRow("COMBO_ADVANCE", stageStart + timing->comboAdvanceMs, HIT_TICK_MS,
                    "PlayerSkills comboAdvanceMs: the earliest Server advance to the next stage; caster hits must end before it.");
            }
        }
        for (const auto& collider : m_CombatRows)
        {
            if (collider.iSkillId != m_SkillId || collider.iStageIndex != stage) continue;
            const auto start = stageStart + collider.iProjectileStartMs + collider.iTimeMs;
            const auto duration = (std::max)(HIT_TICK_MS, collider.iRepeatMs * (collider.iRepeatCount > 0u ? collider.iRepeatCount - 1u : 0u) + HIT_TICK_MS);
            const auto owner = collider.iProjectileIndex == UINT32_MAX ? "Caster" : "Projectile " + std::to_string(collider.iProjectileIndex + 1u);
            const auto detail = owner + (collider.bContact ? " | CONTACT: dynamic overlap" : " | TIMED") + " | " +
                collider.strColliderId + " -> " + collider.strLogicId + " -> " + collider.strResultId;
            ROW colliderRow{ROW_KIND::COLLIDER, collider.strColliderId, "Collider | " + collider.strResultKind, {}, {}, detail, start, duration, stageId, LANE::COLLIDER};
            colliderRow.editable = !collider.bContact;
            rows.push_back(std::move(colliderRow));
            rows.push_back({ROW_KIND::LOGIC, collider.strLogicId, "Logic | AREA_OVERLAP", {}, {}, detail, start, duration, stageId, LANE::LOGIC});
            rows.push_back({ROW_KIND::RESULT, collider.strResultId, "Result | " + collider.strResultKind, {}, {}, detail, start, duration, stageId, LANE::LOGIC});
        }
    }
    if (skill && skill->iCooldownMs > 0u && lastActionEnd > 0u)
    {
        ROW row{ROW_KIND::TIMING, "character.timing.cooldown", "Timing | COOLDOWN " + std::to_string(skill->iCooldownMs) + " ms", {}, {},
            "PlayerSkills cooldownMs after the action lock ends. The Server replicates the cooldown end tick to the HUD.",
            lastActionEnd, skill->iCooldownMs, lastStage, LANE::LOGIC};
        rows.push_back(std::move(row));
    }
    for (const auto& row : rows)
    {
        canvas = (std::max)(canvas, row.start + row.duration);
        // The cooldown box widens the canvas but not the seekable preview range.
        if (row.id != "character.timing.cooldown") end = (std::max)(end, row.start + row.duration);
    }
    m_Rows = std::move(rows); m_StageSummaries = std::move(summaries);
    m_Duration = end; m_CanvasMs = canvas; m_RowsDirty = false;
    if (timingMissing) m_Status = "PlayerSkills.json timing is unavailable for this action; Stage lock, Post-delay and Timing boxes are omitted.";
    return true;
}

bool CCharacterActionWorkbench::Prepare_Preview()
{
    if (!m_Sequencer || !Target_IsCurrent() || !Selected_Binding())
    { m_Status = "Select an action with its admitted preview model before Play."; return false; }
    m_CueEditorClip.clear();
    if (!m_PreviewDirty) return true;
    if (m_Sequencer->Is_Active()) m_PreviewClock = m_Sequencer->ClockMs();
    if (!m_Sequencer->Stage_CharacterAction(m_Asset, *Selected_Binding(), m_Cues, m_CombatRows, m_Stage))
    { m_Status = m_Sequencer->Status(); return false; }
    m_PreviewClock = (std::min)(m_PreviewClock, m_Sequencer->Preview_DurationMs());
    if (m_PreviewClock != 0u && !m_Sequencer->Seek(m_PreviewClock))
    { m_Status = m_Sequencer->Status(); return false; }
    m_PreviewDirty = false;
    return true;
}
void CCharacterActionWorkbench::Render_Transport()
{
    if (ImGui::Button("Play##CharacterAction") && Prepare_Preview())
    {
        if (m_Sequencer->ClockMs() >= m_Sequencer->Preview_DurationMs()) m_Sequencer->Seek(0u);
        m_Sequencer->Play(); m_Status = m_Sequencer->Status();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_Sequencer || !m_Sequencer->Is_Active());
    if (ImGui::Button(m_Sequencer && m_Sequencer->Is_Paused() ? "Resume##CharacterAction" : "Pause##CharacterAction") && m_Sequencer)
        m_Sequencer->Pause(!m_Sequencer->Is_Paused());
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Step##CharacterAction") && Prepare_Preview()) m_Sequencer->Seek(m_Sequencer->ClockMs() + 33u);
    ImGui::SameLine();
    if (ImGui::Button("Stop##CharacterAction")) On_WorkbenchDeactivated();
    ImGui::SameLine();
    if (ImGui::Button("Reload##CharacterAction")) m_Reload = true;
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_BindingsDirty);
    if (ImGui::Button("Save Animation")) m_SaveBindings = true;
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_CombatDirty);
    if (ImGui::Button("Save Combat")) m_SaveCombat = true;
    ImGui::EndDisabled();
    if (m_BindingsDirty || m_CombatDirty) { ImGui::SameLine(); ImGui::TextDisabled("Draft"); }
    else if (m_CombatPublishPending) { ImGui::SameLine(); ImGui::TextColored(WARNING_TEXT, "Combat saved: publish + Server restart pending"); }
}

void CCharacterActionWorkbench::Render_Timeline()
{
    if (!Selected_Binding()) { ImGui::TextWrapped("Select a Character action or its Parent to see the Stage, Animation, Logic, Effect, Collider, Sound and Camera lanes."); return; }
    Render_Transport();
    if (m_Sequencer && !m_PreviewDirty) m_Duration = (std::max)(m_Duration, m_Sequencer->Preview_DurationMs());
    int clock = static_cast<int>(m_PreviewDirty ? m_PreviewClock : m_Sequencer ? m_Sequencer->ClockMs() : 0u);
    ImGui::SetNextItemWidth(280.f);
    if (ImGui::SliderInt("Time##Character", &clock, 0, static_cast<int>(m_Duration), "%d ms") && Prepare_Preview())
        m_Sequencer->Seek(static_cast<std::uint32_t>(clock));
    ImGui::SameLine(); ImGui::SetNextItemWidth(150.f); ImGui::SliderFloat("Zoom##Character", &m_Zoom, 20.f, 400.f, "%.0f px/s");
    ImGui::TextDisabled("Animation: drag reorders, edges trim source. Collider: drag moves the hit time, right edge stretches the repeat run. Sound: drag moves the .animevents cue. Stage / Post-delay / Timing mirror PlayerSkills (read-only).");
    if (ImGui::BeginChild("CharacterActionTimeline", {0.f, 0.f}, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
    {
        constexpr float labelWidth = 120.f, rowHeight = 26.f, rulerHeight = 26.f;
        constexpr std::size_t laneCount = static_cast<std::size_t>(LANE::COUNT);
        const float scale = m_Zoom * .001f;
        const std::uint32_t canvasMs = (std::max)(m_Duration, m_CanvasMs);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = (std::max)(ImGui::GetContentRegionAvail().x, labelWidth + canvasMs * scale + 80.f);
        // Lane layout: fixed Boss lane order, one extra display row per overlap.
        std::array<std::vector<LANE_INTERVAL>, laneCount> intervals;
        const auto minimumMs = static_cast<std::uint64_t>(std::ceil(8.f / scale));
        for (const auto& row : m_Rows)
            intervals[static_cast<std::size_t>(row.lane)].push_back({row.id, row.start,
                row.start + (std::max)(static_cast<std::uint64_t>(row.duration), minimumMs)});
        std::array<std::size_t, laneCount> laneFirstRow{};
        std::unordered_map<std::string, std::size_t> displayRows;
        std::size_t totalRows = 0u;
        for (std::size_t lane = 0u; lane < laneCount; ++lane)
        {
            laneFirstRow[lane] = totalRows;
            totalRows += Allocate_TimelineDisplayRows(std::move(intervals[lane]), displayRows);
        }
        auto* draw = ImGui::GetWindowDrawList();
        const float height = rulerHeight + rowHeight * static_cast<float>(totalRows);
        ImGui::Dummy({width, height});
        CompositionTimeline::DrawRuler(draw, {origin.x + labelWidth, origin.y}, {origin.x + width, origin.y + rulerHeight}, canvasMs, m_Zoom);
        for (std::size_t lane = 0u; lane < laneCount; ++lane)
        {
            const float y = origin.y + rulerHeight + rowHeight * static_cast<float>(laneFirstRow[lane]);
            draw->AddLine({origin.x, y}, {origin.x + width, y}, IM_COL32(60, 64, 72, 255));
            draw->AddText({origin.x + 4.f, y + 5.f}, IM_COL32(200, 204, 212, 255), LANE_LABELS[lane]);
        }
        const bool windowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        const ImVec2 mouse = ImGui::GetMousePos();
        constexpr ImU32 colors[] = {IM_COL32(69,126,203,255), IM_COL32(70,166,111,255), IM_COL32(175,127,60,255), IM_COL32(167,102,174,255),
            IM_COL32(200,96,76,255), IM_COL32(122,121,175,255), IM_COL32(170,102,77,255), IM_COL32(96,96,112,255), IM_COL32(74,84,100,255), IM_COL32(196,118,64,255)};
        static_assert(sizeof(colors) / sizeof(colors[0]) == static_cast<std::size_t>(ROW_KIND::TIMING) + 1u);
        for (const auto& row : m_Rows)
        {
            const auto displayRow = displayRows.find(row.id);
            if (displayRow == displayRows.end()) continue;
            const float y = origin.y + rulerHeight + rowHeight * static_cast<float>(laneFirstRow[static_cast<std::size_t>(row.lane)] + displayRow->second);
            const float startX = origin.x + labelWidth + row.start * scale;
            const float endX = startX + (std::max)(8.f, row.duration * scale);
            const bool selected = m_SelectedRow == row.id;
            const bool animation = row.kind == ROW_KIND::ANIMATION;
            const bool colliderEdge = row.kind == ROW_KIND::COLLIDER && row.editable;
            const ImU32 fill = row.warning ? IM_COL32(150, 82, 82, 255) : colors[static_cast<std::size_t>(row.kind)];
            CompositionTimeline::DrawBox(draw, {startX, y + 1.f}, {endX, y + rowHeight - 3.f}, fill, selected, row.label.c_str(), animation, animation || colliderEdge);
            const bool hover = windowHovered && mouse.x >= startX && mouse.x <= endX && mouse.y >= y && mouse.y <= y + rowHeight;
            if (!hover) continue;
            ImGui::SetTooltip("%s\n%s", row.id.c_str(), row.detail.c_str());
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                m_SelectedRow = row.id;
                if (m_Sequencer) m_Sequencer->Pause(true);
                if (row.editable)
                {
                    m_DragId = row.id; m_DragKind = row.kind; m_DragX = mouse.x;
                    m_DragGesture = static_cast<int>(CompositionTimeline::HitBoxGesture(mouse.x, startX, endX, 5.f, animation, animation || colliderEdge));
                }
            }
        }
        if (clock >= 0 && static_cast<std::uint32_t>(clock) <= canvasMs)
        {
            const float playheadX = origin.x + labelWidth + static_cast<float>(clock) * scale;
            draw->AddLine({playheadX, origin.y}, {playheadX, origin.y + height},
                m_Sequencer && m_Sequencer->Is_Active() ? IM_COL32(255, 220, 72, 230) : IM_COL32(200, 200, 200, 140), 1.5f);
        }
        if (!m_DragId.empty() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            const int deltaMs = static_cast<int>((ImGui::GetMousePos().x - m_DragX) * 1000.f / m_Zoom);
            Apply_BoxGesture(m_DragId, m_DragKind, m_DragGesture, deltaMs); m_DragId.clear();
        }
    }
    ImGui::EndChild();
    if (m_Sequencer) m_Sequencer->Render_PreviewOverlays();
}

void CCharacterActionWorkbench::Apply_BoxGesture(const std::string& id, const ROW_KIND kind, const int gesture, const int deltaMs)
{
    if (!deltaMs) return;
    if (kind == ROW_KIND::COLLIDER)
    {
        const auto found = std::find_if(m_CombatRows.begin(), m_CombatRows.end(), [&](const auto& row) { return row.strColliderId == id; });
        if (found == m_CombatRows.end() || found->bContact) return;
        auto candidate = *found;
        if (gesture == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_END))
        {
            // The right edge stretches the repeat run. An authored interval keeps
            // its spacing and only the count follows; a single hit grows into two.
            const std::int64_t previous = static_cast<std::int64_t>(candidate.iRepeatMs) * (candidate.iRepeatCount > 0u ? candidate.iRepeatCount - 1u : 0u);
            const std::int64_t span = (std::max)(std::int64_t{0}, previous + deltaMs);
            if (candidate.iRepeatCount <= 1u || candidate.iRepeatMs == 0u)
            {
                if (span < static_cast<std::int64_t>(HIT_TICK_MS)) candidate.iRepeatCount = 1u;
                else { candidate.iRepeatCount = 2u; candidate.iRepeatMs = static_cast<std::uint32_t>((std::min)(span, std::int64_t{100000})); }
            }
            else
            {
                const long long count = std::llround(static_cast<double>(span) / candidate.iRepeatMs) + 1ll;
                candidate.iRepeatCount = static_cast<std::uint32_t>((std::clamp)(count, 1ll, 64ll));
            }
        }
        else candidate.iTimeMs = static_cast<std::uint32_t>((std::clamp)(static_cast<int>(candidate.iTimeMs) + deltaMs, 0, static_cast<int>(MAX_EDITOR_TIME_MS)));
        // Repeat count 0 is a rejected Save value; treat it as one hit here so the limit check does not underflow.
        const std::uint64_t finalMs = candidate.iTimeMs + static_cast<std::uint64_t>(candidate.iRepeatCount > 0u ? candidate.iRepeatCount - 1u : 0u) * candidate.iRepeatMs;
        const auto limit = Collider_LimitMs(candidate.iStageIndex);
        if (candidate.iProjectileIndex == UINT32_MAX && finalMs > limit)
        { m_Status = "Collider edit rejected: its last hit would pass the Server action/stage limit of " + std::to_string(limit) + " ms."; return; }
        if (candidate == *found) return;
        *found = std::move(candidate);
        m_CombatDirty = m_RowsDirty = m_PreviewDirty = true;
        return;
    }
    if (kind == ROW_KIND::SOUND)
    {
        const auto found = std::find_if(m_Rows.begin(), m_Rows.end(), [&](const auto& value) { return value.id == id; });
        if (found == m_Rows.end()) return;
        // The lanes run on the action clock; the cue owner stores clip source ms.
        const ROW moved = *found;
        std::uint32_t source = 0u;
        if (!Sound_SourceMs(moved, deltaMs, source))
        { m_Status = "The Sound cue's source clip is no longer part of this action."; return; }
        Apply_SoundEdit(moved, source, moved.asset);
        return;
    }
    auto* clip = Find_Clip(id);
    if (!clip) return;
    if (gesture == static_cast<int>(CompositionTimeline::BoxGesture::MOVE)) { Move_Clip(id, deltaMs > 0 ? 1 : -1); return; }
    auto candidate = *clip;
    std::uint32_t wall = 0u;
    if (!Clip_Duration(candidate, wall)) return;
    const int sourceLength = candidate.iPlayMs ? static_cast<int>(candidate.iPlayMs) : static_cast<int>(std::round(wall * candidate.fPlayRate));
    const int deltaSource = static_cast<int>(std::round(deltaMs * candidate.fPlayRate));
    if (gesture == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START))
    {
        const int start = (std::clamp)(static_cast<int>(candidate.iSourceStartMs) + deltaSource, 0, static_cast<int>(candidate.iSourceStartMs) + sourceLength - 1);
        candidate.iPlayMs = candidate.iSourceStartMs + sourceLength - start; candidate.iSourceStartMs = start;
    }
    else candidate.iPlayMs = static_cast<std::uint32_t>((std::clamp)(sourceLength + deltaSource, 1, 60000));
    if (!Clip_Duration(candidate, wall)) { m_Status = "Trim rejected because it leaves the actual model source window."; return; }
    *clip = std::move(candidate); m_BindingsDirty = m_RowsDirty = m_PreviewDirty = true;
}
void CCharacterActionWorkbench::Move_Clip(const std::string& id, const int direction)
{
    if (auto* binding = Selected_Binding())
        for (auto& stage : binding->Stages)
            for (std::size_t i = 0u; i < stage.Clips.size(); ++i)
                if (stage.Clips[i].strClipOccurrenceId == id)
                {
                    const auto target = static_cast<int>(i) + direction;
                    if (target < 0 || target >= static_cast<int>(stage.Clips.size())) return;
                    std::swap(stage.Clips[i], stage.Clips[target]);
                    m_BindingsDirty = m_RowsDirty = m_PreviewDirty = true; return;
                }
}

void CCharacterActionWorkbench::Render_Details()
{
    const auto found = std::find_if(m_Rows.begin(), m_Rows.end(), [&](const auto& row) { return row.id == m_SelectedRow; });
    if (found == m_Rows.end()) { ImGui::TextDisabled("Select a Stage, Animation, Timing, Effect, Collider, Logic, Result, Sound or Camera box."); return; }
    const ROW row = *found;
    ImGui::TextWrapped("%s", row.label.c_str());
    ImGui::TextWrapped("%s", row.id.c_str());
    ImGui::Text("Stage %u | %u + %u ms", row.stage + 1u, row.start, row.duration);
    ImGui::TextWrapped("%s", row.detail.c_str());
    switch (row.kind)
    {
    case ROW_KIND::ANIMATION: Render_AnimationDetail(row); break;
    case ROW_KIND::COLLIDER: case ROW_KIND::LOGIC: case ROW_KIND::RESULT: Render_CombatDetail(row); break;
    case ROW_KIND::STAGE: case ROW_KIND::GAP: case ROW_KIND::TIMING: Render_StageDetail(row); break;
    case ROW_KIND::SOUND: Render_SoundDetail(row); break;
    default:
        if (!row.asset.empty() && m_OpenEffect && ImGui::Button("Edit Effect resource")) m_OpenEffect(row.asset);
        Render_CueOwnerSection(row);
        break;
    }
}
void CCharacterActionWorkbench::Render_CueOwnerSection(const ROW& row)
{
    ImGui::SeparatorText("Animation cue owner");
    ImGui::TextWrapped("Edit source cue timing, anchor and transform below; Save writes the same Product .animevents owner. Refresh action cues after saving.");
    if (ImGui::Button("Refresh saved cues") && Refresh_Cues())
        m_Status = "Reloaded the saved Animation cue rows.";
    Render_CueEditor(row);
}

void CCharacterActionWorkbench::Render_AnimationDetail(const ROW& row)
{
    auto* clip = Find_Clip(row.id);
    if (!clip) return;
    auto candidate = *clip;
    bool changed = Edit_U32("Source start (ms)", candidate.iSourceStartMs, 60000);
    changed |= Edit_U32("Source length (0 = remainder)", candidate.iPlayMs, 60000);
    changed |= ImGui::DragFloat("Playback rate", &candidate.fPlayRate, .01f, .05f, 16.f, "%.3f");
    if (changed)
    {
        std::uint32_t duration;
        if (Clip_Duration(candidate, duration)) { *clip = candidate; m_BindingsDirty = m_RowsDirty = m_PreviewDirty = true; }
        else m_Status = "The requested source window exceeds this model clip; the prior values were kept.";
    }
    if (std::uint32_t wall = 0u; Clip_Duration(*clip, wall))
    {
        // Derived like Rebuild_Rows' source window; playMs == 0 resolves to the clip remainder.
        const auto sourceOut = clip->iSourceStartMs + static_cast<std::uint32_t>(std::ceil(wall * clip->fPlayRate));
        ImGui::Text("Source out: %u ms (derived) | wall length: %u ms", sourceOut, wall);
    }
    ImGui::TextWrapped("The source length is divided by playback rate. Server action duration and judgement timing remain separately authored.");
    if (ImGui::Button("Earlier")) Move_Clip(row.id, -1);
    ImGui::SameLine(); if (ImGui::Button("Later")) Move_Clip(row.id, 1);
    ImGui::SameLine();
    if (ImGui::Button("Duplicate"))
    {
        if (auto* binding = Selected_Binding())
            for (auto& stage : binding->Stages)
                for (std::size_t i = 0u; i < stage.Clips.size(); ++i)
                    if (stage.Clips[i].strClipOccurrenceId == row.id)
                    {
                        auto copy = stage.Clips[i]; copy.strClipOccurrenceId = CEffectEditingSession::New_Id("character.clip.");
                        stage.Clips.insert(stage.Clips.begin() + i + 1u, std::move(copy));
                        m_BindingsDirty = m_RowsDirty = m_PreviewDirty = true; return;
                    }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove"))
    {
        if (auto* binding = Selected_Binding())
            for (auto& stage : binding->Stages)
            {
                const auto found = std::find_if(stage.Clips.begin(), stage.Clips.end(), [&](const auto& value) { return value.strClipOccurrenceId == row.id; });
                if (found == stage.Clips.end()) continue;
                if (stage.Clips.size() == 1u) { m_Status = "Every Server stage needs at least one animation clip."; return; }
                stage.Clips.erase(found); m_BindingsDirty = m_RowsDirty = m_PreviewDirty = true; m_SelectedRow.clear(); return;
            }
    }
    if (ImGui::Button("Save Animation##Detail")) m_SaveBindings = true;
    if (ImGui::CollapsingHeader("Edit this clip's cues")) Render_CueEditor(row);
}

void CCharacterActionWorkbench::Render_StageDetail(const ROW& row)
{
    ImGui::SeparatorText(row.kind == ROW_KIND::TIMING ? "Server timing (read-only)" : "Server stage (read-only)");
    const STAGE_SUMMARY* summary = row.stage < m_StageSummaries.size() ? &m_StageSummaries[row.stage] : nullptr;
    if (summary)
    {
        ImGui::Text("Clip wall end: %u ms", summary->clipEndMs);
        if (!summary->hasTiming) ImGui::TextDisabled("PlayerSkills.json timing is unavailable for this stage.");
        else
        {
            ImGui::Text("Server actionDurationMs: %u ms", summary->actionDurationMs);
            if (summary->actionDurationMs >= summary->clipEndMs)
                ImGui::Text("Post-delay: %u ms", summary->actionDurationMs - summary->clipEndMs);
            else
                ImGui::TextColored(WARNING_TEXT, "Clips overrun the Server action by %u ms; the publisher clamps hit times to actionDurationMs.",
                    summary->clipEndMs - summary->actionDurationMs);
        }
    }
    if (const auto* timing = Stage_Timing(row.stage))
    {
        ImGui::Text("hitTimeMs: %u", timing->hitTimeMs);
        if (timing->combo) ImGui::Text("comboAdvanceMs: %u | input window: %u .. %u ms", timing->comboAdvanceMs, timing->inputOpenMs, timing->inputCloseMs);
        ImGui::Text("Caster hit limit for Save Combat: %u ms", Collider_LimitMs(row.stage));
    }
    if (const auto* skill = CPlayerSkillCatalog::Find_ById(m_SkillId)) ImGui::Text("cooldownMs: %u", skill->iCooldownMs);
    ImGui::TextWrapped("actionDurationMs and the other timing fields are Server truth in Data/Balance/PlayerSkills.json. Edit them in the F1 Balance Tool, run Publish-GameplayBalance.ps1 and restart Server; this Workbench never writes PlayerSkills.json. Save Animation changes only the presentation clips.");
}

void CCharacterActionWorkbench::Render_SoundDetail(const ROW& row)
{
    ImGui::SeparatorText("Sound cue (.animevents SOUND)");
    ImGui::Text("Event: %s", row.asset.c_str());
    ImGui::Text("Action clock: %u ms | source clip %s at %u ms", row.start, row.clip.c_str(), row.source);
    const auto found = m_SoundEvents.find(row.asset);
    if (found == m_SoundEvents.end()) ImGui::TextColored(WARNING_TEXT, "CharacterSoundCatalog.json has no event row for this class: %s", m_SoundStatus.c_str());
    else if (found->second.empty()) ImGui::TextColored(WARNING_TEXT, "Unresolved: the catalog row has no wav variants yet, so runtime skips this cue.");
    else
    {
        ImGui::Text("%zu equally weighted variant(s):", found->second.size());
        for (const auto& variant : found->second) ImGui::BulletText("%s", variant.c_str());
        if (ImGui::Button("Preview##Sound")) Preview_Sound(row.asset);
        ImGui::SameLine();
        ImGui::BeginDisabled(m_SoundPreviewHandle == 0u);
        if (ImGui::Button("Stop preview##Sound")) Stop_SoundPreview();
        ImGui::EndDisabled();
    }
    // One owner: CAnimation_Tool validates and replaces the .animevents document
    // atomically, and refuses while its own cue draft is unsaved.
    if (m_SoundEditRow != row.id) { m_SoundEditRow = row.id; m_SoundEditEvent = row.asset; m_SoundEditStartMs = row.source; }
    if (ImGui::BeginCombo("Event##SoundEdit", m_SoundEditEvent.c_str()))
    {
        for (const auto& name : m_SoundEventNames)
            if (ImGui::Selectable(name.c_str(), name == m_SoundEditEvent)) m_SoundEditEvent = name;
        ImGui::EndCombo();
    }
    if (m_SoundEventNames.empty()) ImGui::TextDisabled("The class sound catalog is unavailable, so only the start time can be changed here.");
    Edit_U32("Source start (ms)##SoundEdit", m_SoundEditStartMs, 60000);
    ImGui::BeginDisabled(!m_AnimationTool || !Target_IsCurrent());
    if (ImGui::Button("Apply##SoundEdit")) Apply_SoundEdit(row, m_SoundEditStartMs, m_SoundEditEvent);
    ImGui::SameLine();
    if (ImGui::Button("Remove##SoundEdit")) Remove_Sound(row);
    ImGui::EndDisabled();
    ImGui::TextWrapped("Dragging the Sound box moves the same row. Apply and Remove rewrite the Product .animevents owner immediately and the rows below rebuild from the saved file; Server gameplay is unaffected.");
    Render_CueOwnerSection(row);
}

void CCharacterActionWorkbench::Render_CueEditor(const ROW& row)
{
    if (!m_AnimationTool) return;
    const bool enter = ImGui::Button("Edit source cues");
    if (enter)
    {
        // Paused sequencers still sample the model each frame. Release their
        // clock before explicitly handing the source clip to its cue editor.
        On_WorkbenchDeactivated();
        m_CueEditorClip = row.clip;
    }
    if (m_CueEditorClip != row.clip || (m_Sequencer && m_Sequencer->Is_Active()))
    {
        ImGui::TextWrapped("Open source cue editing to pause the action and select this source clip. Play or Seek returns to the action timeline.");
        return;
    }
    if (!m_AnimationTool->Render_CharacterActionCueEditor(m_Asset, m_Generation, row.clip, m_CueStatus, enter))
        m_CueEditorClip.clear();
}

void CCharacterActionWorkbench::Render_CombatDetail(const ROW& row)
{
    const auto found = std::find_if(m_CombatRows.begin(), m_CombatRows.end(), [&](const auto& value) {
        return value.strColliderId == row.id || value.strLogicId == row.id || value.strResultId == row.id; });
    if (found == m_CombatRows.end()) return;
    auto& value = *found;
    ImGui::SeparatorText("Collider -> Logic -> Result");
    ImGui::TextWrapped("%s\nAREA_OVERLAP: %s\n%s: %s", value.strColliderId.c_str(), value.strLogicId.c_str(), value.strResultKind.c_str(), value.strResultId.c_str());
    if (const auto* skill = CPlayerSkillCatalog::Find_ById(value.iSkillId))
    {
        if (value.strResultKind == "DAMAGE") ImGui::Text("Damage: %u%% attack power", skill->iDamageRatePercent);
        else if (value.strResultKind == "STAGGER") ImGui::Text("Stagger: %u", skill->iStaggerDamage);
        else if (value.strResultKind == "COUNTER") ImGui::Text("Counter power: %u", skill->iCounterPower);
    }
    ImGui::TextWrapped("Result power comes from PlayerSkills. Each collider submits only its connected Result on the Server.");
    bool changed = false;
    ImGui::BeginDisabled(value.bContact);
    changed |= Edit_U32(value.iProjectileIndex == UINT32_MAX ? "Action / stage time (ms)" : "Projectile local time (ms)", value.iTimeMs);
    ImGui::EndDisabled();
    if (value.iProjectileIndex != UINT32_MAX)
    {
        ImGui::Text("Owner: Projectile %u | starts at %u ms", value.iProjectileIndex + 1u, value.iProjectileStartMs);
        ImGui::TextWrapped("The wire shows the authored shape at the caster root. Projectile travel and spatial contact are evaluated by the Server.");
    }
    else
    {
        ImGui::TextUnformatted("Owner: Caster");
        const auto limit = Collider_LimitMs(value.iStageIndex);
        const std::uint64_t finalMs = value.iTimeMs + static_cast<std::uint64_t>(value.iRepeatCount > 0u ? value.iRepeatCount - 1u : 0u) * value.iRepeatMs;
        if (finalMs > limit) ImGui::TextColored(WARNING_TEXT, "Last hit at %llu ms passes the Server limit of %u ms; Save Combat will reject it.", static_cast<unsigned long long>(finalMs), limit);
        else ImGui::Text("Last hit at %llu ms | Server limit %u ms", static_cast<unsigned long long>(finalMs), limit);
    }
    if (value.bContact) ImGui::TextWrapped("CONTACT has no fixed hit time. Its box marks the projectile start; the Server triggers the result when contact occurs.");
    changed |= Edit_U32("Repeat count", value.iRepeatCount, 64);
    changed |= Edit_U32("Repeat interval (ms)", value.iRepeatMs, 100000);
    changed |= Edit_Double("Range (m)", value.fRange, .001f, 1000.f);
    changed |= Edit_Double("Width (m)", value.fWidth, 0.f, 1000.f);
    changed |= Edit_Double("Height (m)", value.fHeight, 0.f, 1000.f);
    changed |= Edit_Double("Forward offset (m)", value.fOffset, -1000.f, 1000.f);
    changed |= Edit_Double("Inner radius (m)", value.fInner, 0.f, 1000.f);
    changed |= Edit_Double("Fan angle (degrees)", value.fAngleDegrees, 0.f, 360.f);
    int shape = static_cast<int>(value.iAreaType) - 1;
    if (ImGui::Combo("Shape", &shape, "Circle / Ring\0Box\0Fan\0")) { value.iAreaType = static_cast<std::uint32_t>(shape + 1); changed = true; }
    if (changed) m_CombatDirty = m_RowsDirty = m_PreviewDirty = true;
    if (ImGui::Button("Save Combat##Detail")) m_SaveCombat = true;
    ImGui::SameLine();
    ImGui::BeginDisabled(value.iProjectileIndex != UINT32_MAX);
    if (ImGui::Button("Remove Collider")) m_RemoveCollider = value.strColliderId;
    ImGui::EndDisabled();
    Help_Marker("Retires this Collider / Logic / Result triplet in the HitShapes owner. The last hit of a stage without projectiles stays, because the publisher rejects an empty stage. Save Combat writes the change.");
    ImGui::TextWrapped("Save updates HitShapes authoring only.");
    if (m_CombatPublishPending)
        ImGui::TextColored(WARNING_TEXT, "Saved HitShapes await Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish and a Server restart.");
    else ImGui::TextWrapped("Runtime needs Publish-GameplayBalance.ps1 -Mode Publish and a Server restart after Save.");
}

void CCharacterActionWorkbench::Render_Resources()
{
    if (!ImGui::BeginTabBar("##CharacterResourceCategories")) return;
    for (const char* category : RESOURCE_CATEGORIES)
    {
        if (!ImGui::BeginTabItem(category)) continue;
        const std::string_view name = category;
        if (name == "Animation") Render_AnimationResources();
        else if (name == "Logic") Render_LogicResources();
        else if (name == "Effect") Render_EffectResources();
        else if (name == "Collider") Render_ColliderResources();
        else if (name == "Sound") Render_SoundResources();
        else if (name == "Camera") Render_CameraResources();
        else Render_PatternResources();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
void CCharacterActionWorkbench::Render_AnimationResources()
{
    ImGui::Text("%s | %zu actual model clips", m_Asset.c_str(), m_ClipNames.size());
    for (const auto& name : m_ClipNames)
    {
        if (ImGui::Selectable(name.c_str(), m_SelectedResource == name)) m_SelectedResource = name;
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            COMPOSITION_ANIMATION_RESOURCE resource; resource.strTargetAssetName = m_Asset; resource.strRuntimeClip = name;
            Append_CompositionAnimationResource(resource, false, m_Status);
        }
    }
}
void CCharacterActionWorkbench::Render_LogicResources()
{
    const auto* skill = CPlayerSkillCatalog::Find_ById(m_SkillId);
    if (!skill || !Selected_Binding()) { ImGui::TextDisabled("Select an action to list its Server timing and hit logic."); return; }
    ImGui::SeparatorText("Server timing (PlayerSkills.json, read-only)");
    ImGui::Text("cooldownMs: %u", skill->iCooldownMs);
    const auto found = m_SkillTimings.find(m_SkillId);
    if (found == m_SkillTimings.end()) ImGui::TextDisabled("Timing rows are unavailable; fix PlayerSkills.json and Reload.");
    else if (ImGui::BeginTable("##CharacterTiming", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Stage"); ImGui::TableSetupColumn("actionDurationMs"); ImGui::TableSetupColumn("hitTimeMs");
        ImGui::TableSetupColumn("comboAdvanceMs"); ImGui::TableSetupColumn("inputOpenMs"); ImGui::TableSetupColumn("inputCloseMs");
        ImGui::TableHeadersRow();
        for (std::size_t i = 0u; i < found->second.size(); ++i)
        {
            const auto& timing = found->second[i];
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("%zu", i + 1u);
            ImGui::TableNextColumn(); ImGui::Text("%u", timing.actionDurationMs);
            ImGui::TableNextColumn(); ImGui::Text("%u", timing.hitTimeMs);
            if (timing.combo)
            {
                ImGui::TableNextColumn(); ImGui::Text("%u", timing.comboAdvanceMs);
                ImGui::TableNextColumn(); ImGui::Text("%u", timing.inputOpenMs);
                ImGui::TableNextColumn(); ImGui::Text("%u", timing.inputCloseMs);
            }
            else for (int column = 0; column < 3; ++column) { ImGui::TableNextColumn(); ImGui::TextDisabled("-"); }
        }
        ImGui::EndTable();
    }
    ImGui::TextWrapped("Edit these in the F1 Balance Tool; Publish-GameplayBalance.ps1 and a Server restart apply them. The timeline Logic lane mirrors them as Timing boxes.");
    ImGui::SeparatorText("Hit logic (HitShapes, DURATION / AREA_OVERLAP)");
    std::size_t count = 0u;
    for (const auto& value : m_CombatRows)
    {
        if (value.iSkillId != m_SkillId) continue;
        ++count;
        const std::string label = value.strLogicId + " -> " + value.strResultKind;
        if (ImGui::Selectable(label.c_str(), m_SelectedRow == value.strLogicId)) m_SelectedRow = value.strLogicId;
    }
    if (!count) ImGui::TextDisabled("This skill has no HitShapes rows; the Server uses hitTimeMs and maximumRange.");
}
void CCharacterActionWorkbench::Render_EffectResources()
{
    if (!m_EffectInventoryLoaded || ImGui::Button("Refresh Effect resources"))
    {
        std::vector<CEffectAuthoringResourceTree::RESOURCE> resources;
        if (CEffectAuthoringResourceTree::Read_V1Inventory(resources, m_Status))
        {
            std::vector<std::string> ids;
            for (const auto& resource : resources) ids.push_back(resource.strAssetId);
            m_EffectIds = std::move(ids); m_EffectInventoryLoaded = true;
        }
    }
    std::set<std::string> linked;
    for (const auto& row : m_Rows) if (row.kind == ROW_KIND::EFFECT) linked.insert(row.asset);
    for (const auto& id : m_EffectIds)
    {
        const std::string label = (linked.contains(id) ? "[Linked] " : "") + id;
        if (ImGui::Selectable(label.c_str(), m_SelectedResource == id)) m_SelectedResource = id;
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && m_OpenEffect) m_OpenEffect(id);
    }
}
void CCharacterActionWorkbench::Render_ColliderResources()
{
    std::size_t count = 0u;
    for (const auto& value : m_CombatRows)
    {
        if (value.iSkillId != m_SkillId) continue;
        ++count;
        const std::string label = value.strResultKind + " | " + value.strColliderId;
        if (ImGui::Selectable(label.c_str(), m_SelectedRow == value.strColliderId)) m_SelectedRow = value.strColliderId;
    }
    if (!count) ImGui::TextDisabled("This skill has no HitShapes colliders.");
    ImGui::Separator();
    ImGui::BeginDisabled(!Selected_Binding());
    if (ImGui::Button("Add Collider")) m_AddCollider = true;
    ImGui::EndDisabled();
    Help_Marker("Creates skill<id>.stage<n>.caster.hit<k>.<result> in the HitShapes owner at the playhead of the selected stage. A selected collider is copied as the shape prototype; otherwise the row defaults are used. Save Combat writes it.");
}
void CCharacterActionWorkbench::Render_SoundResources()
{
    ImGui::Text("%s | %zu catalog events", m_Asset.c_str(), m_SoundEventNames.size());
    if (m_SoundEventNames.empty()) ImGui::TextWrapped("%s", m_SoundStatus.c_str());
    std::set<std::string> linked;
    for (const auto& row : m_Rows) if (row.kind == ROW_KIND::SOUND) linked.insert(row.asset);
    for (const auto& name : m_SoundEventNames)
    {
        const auto variants = m_SoundEvents.find(name);
        const std::size_t variantCount = variants == m_SoundEvents.end() ? 0u : variants->second.size();
        const std::string label = (linked.contains(name) ? "[Linked] " : "") + name +
            (variantCount ? " (" + std::to_string(variantCount) + ")" : " (unresolved)");
        if (ImGui::Selectable(label.c_str(), m_SelectedResource == name)) m_SelectedResource = name;
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) Preview_Sound(name);
    }
    ImGui::Separator();
    ImGui::BeginDisabled(m_SelectedResource.empty() || !m_AnimationTool || !Target_IsCurrent());
    if (ImGui::Button("Add Sound at playhead")) Add_SoundAtPlayhead(m_SelectedResource);
    ImGui::EndDisabled();
    Help_Marker("The selected catalog event becomes a SOUND row on the Animation box under the playhead, at that box's own source time. CAnimation_Tool writes the .animevents owner atomically.");
    ImGui::TextWrapped("Double-click previews the first wav variant. Select an event and Add Sound at playhead, then move or retire it from the Sound box in Box Detail.");
}
void CCharacterActionWorkbench::Render_CameraResources()
{
    ImGui::SeparatorText("Camera shake cues (.animevents SHAKE, read-only)");
    std::size_t count = 0u;
    for (const auto& row : m_Rows)
    {
        if (row.kind != ROW_KIND::SHAKE) continue;
        const auto cue = std::find_if(m_Cues.Shakes.begin(), m_Cues.Shakes.end(),
            [&](const auto& value) { return value.strClipName == row.clip && value.iStartMs == row.source; });
        if (cue == m_Cues.Shakes.end()) continue;
        ++count;
        char label[256]{};
        std::snprintf(label, sizeof(label), "%s @ %u ms | %.2fs (in %.2f / out %.2f) | fwd %.2f right %.2f up %.2f fov %.2f",
            row.clip.c_str(), row.source, cue->Spec.fDurationSeconds, cue->Spec.fBlendInSeconds, cue->Spec.fBlendOutSeconds,
            cue->Spec.Forward.fAmplitude, cue->Spec.Right.fAmplitude, cue->Spec.Up.fAmplitude, cue->Spec.Fov.fAmplitude);
        if (ImGui::Selectable(label, m_SelectedRow == row.id)) m_SelectedRow = row.id;
    }
    if (!count) ImGui::TextDisabled("The selected action has no SHAKE rows inside its clip source windows.");
    ImGui::TextWrapped("Camera shakes are Character presentation only; the payload is authored on the SHAKE row of the source cue editor.");
}
void CCharacterActionWorkbench::Render_PatternResources()
{
    const auto* skill = CPlayerSkillCatalog::Find_ById(m_SkillId);
    if (!skill || m_ClassIndex < 0 || !Selected_Binding()) { ImGui::TextDisabled("Select an action in Composition Actions."); return; }
    ImGui::Text("Class: %s (%s)", CLASSES[m_ClassIndex].label, m_Asset.c_str());
    ImGui::Text("Action: %s | %s", skill->strActionId.c_str(), skill->strDisplayName.c_str());
    ImGui::Text("Skill %u | slot %s | %s | %zu Server stage(s)", skill->iSkillId, skill->strInputSlot.c_str(),
        Skill_KindLabel(skill->eSkillKind), Selected_Binding()->Stages.size());
    if (m_Stage) ImGui::Text("Selected stage: %u", *m_Stage + 1u); else ImGui::TextUnformatted("All stages");
    ImGui::TextWrapped("A player skill is the Character pattern unit. Its id, slot and stage count come from PlayerSkills.json and cannot be created or renamed here.");
}

bool CCharacterActionWorkbench::Preview_Sound(const std::string& eventName)
{
    Stop_SoundPreview();
    const auto found = m_SoundEvents.find(eventName);
    if (found == m_SoundEvents.end() || found->second.empty()) { m_Status = "No wav variant is resolved for " + eventName; return false; }
    const auto path = CRuntimeAssetRoot::Resolve(found->second.front());
    if (path.empty()) { m_Status = "Cannot resolve sound asset: " + found->second.front(); return false; }
    m_SoundPreviewHandle = CGameInstance::Get().Play_SoundCue(path.wstring(), 1.f);
    if (!m_SoundPreviewHandle) { m_Status = "Sound preview failed to start: " + found->second.front(); return false; }
    m_Status = "Previewing " + found->second.front();
    return true;
}
void CCharacterActionWorkbench::Stop_SoundPreview()
{
    if (!m_SoundPreviewHandle) return;
    CGameInstance::Get().Stop_SoundCue(m_SoundPreviewHandle);
    m_SoundPreviewHandle = 0u;
}

bool CCharacterActionWorkbench::Can_AppendCompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
    const bool replace, std::string& status) const
{
    if (!Target_IsCurrent() || !Selected_Binding() || resource.strTargetAssetName != m_Asset ||
        std::find(m_ClipNames.begin(), m_ClipNames.end(), resource.strRuntimeClip) == m_ClipNames.end())
    { status = "Select an action on this exact Character model before appending its clip."; return false; }
    if (replace && !const_cast<CCharacterActionWorkbench*>(this)->Find_Clip(m_SelectedRow))
    { status = "Select the Animation box to replace."; return false; }
    if (!replace)
    {
        std::size_t count = 0u;
        for (const auto& stage : Selected_Binding()->Stages) count += stage.Clips.size();
        if (count >= 16u) { status = "An action supports at most 16 animation clips."; return false; }
    }
    if (!replace && !m_Stage && Selected_Binding()->Stages.size() > 1u)
    { status = "Select one Server stage before appending a clip; stage count is fixed by PlayerSkills."; return false; }
    return true;
}
bool CCharacterActionWorkbench::Append_CompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
    const bool replace, std::string& status)
{
    if (!Can_AppendCompositionAnimationResource(resource, replace, status)) return false;
    if (replace)
    {
        auto* current = Find_Clip(m_SelectedRow);
        auto candidate = *current; candidate.strClipName = resource.strRuntimeClip;
        std::uint32_t duration = 0u;
        if (!Clip_Duration(candidate, duration)) { status = "Replacement does not contain the selected source window; the original clip was preserved."; return false; }
        *current = std::move(candidate);
    }
    else
    {
        auto* binding = Selected_Binding();
        ANIMATION_SKILL_CLIP clip; clip.strClipName = resource.strRuntimeClip;
        clip.strClipOccurrenceId = CEffectEditingSession::New_Id("character.clip.");
        binding->Stages[m_Stage.value_or(0u)].Clips.push_back(std::move(clip));
    }
    m_BindingsDirty = m_RowsDirty = m_PreviewDirty = true;
    status = "Animation binding draft updated. Save Animation writes the Product skill binding.";
    return true;
}

std::uint32_t CCharacterActionWorkbench::Playhead_Ms() const
{
    return m_Sequencer && !m_PreviewDirty ? m_Sequencer->ClockMs() : m_PreviewClock;
}
bool CCharacterActionWorkbench::Add_Collider()
{
    const auto* binding = Selected_Binding();
    if (!binding || m_ClassIndex < 0) { m_Status = "Select a Character action before adding a collider."; return false; }
    if (!m_Stage && binding->Stages.size() > 1u)
    { m_Status = "Select one Server stage before adding a collider; the stage count is fixed by PlayerSkills."; return false; }
    const std::uint32_t stage = m_Stage.value_or(0u);
    // A selected collider is the shape prototype; otherwise the row defaults apply.
    CHARACTER_ACTION_COMBAT_ROW prototype;
    const auto selected = std::find_if(m_CombatRows.begin(), m_CombatRows.end(), [&](const auto& value) {
        return value.strColliderId == m_SelectedRow || value.strLogicId == m_SelectedRow || value.strResultId == m_SelectedRow; });
    if (selected != m_CombatRows.end() && selected->iSkillId == m_SkillId) prototype = *selected;
    const std::uint32_t stageStart = stage < m_StageSummaries.size() ? m_StageSummaries[stage].startMs : 0u;
    const std::uint32_t clock = Playhead_Ms();
    prototype.iTimeMs = clock > stageStart ? (std::min)(clock - stageStart, Collider_LimitMs(stage)) : 0u;
    std::string created;
    if (!m_Combat.Insert_CasterHit(m_SkillId, stage, prototype, created, m_Status)) return false;
    const auto& rows = m_Combat.Get_Rows();
    const auto row = std::find_if(rows.begin(), rows.end(), [&](const auto& value) { return value.strColliderId == created; });
    if (row == rows.end()) { m_Status = "The combat owner did not return the created collider."; return false; }
    m_CombatRows.push_back(*row);
    m_SelectedRow = created;
    m_CombatDirty = m_RowsDirty = m_PreviewDirty = true;
    return true;
}
bool CCharacterActionWorkbench::Remove_Collider(const std::string& colliderId)
{
    if (!m_Combat.Remove_CasterHit(colliderId, m_Status)) return false;
    const auto found = std::find_if(m_CombatRows.begin(), m_CombatRows.end(),
        [&](const auto& value) { return value.strColliderId == colliderId; });
    if (found != m_CombatRows.end())
    {
        if (m_SelectedRow == found->strColliderId || m_SelectedRow == found->strLogicId || m_SelectedRow == found->strResultId)
            m_SelectedRow.clear();
        m_CombatRows.erase(found);
    }
    m_CombatDirty = m_RowsDirty = m_PreviewDirty = true;
    return true;
}
bool CCharacterActionWorkbench::Refresh_Cues()
{
    ANIMATION_EFFECT_CUE_DOCUMENT staged;
    std::string status;
    if (!CAnimationEffectCueDocument::Load(m_Asset, m_ClipNames, staged, status))
    { m_Status += " The saved cue document could not be re-read: " + status; return false; }
    m_Cues = std::move(staged); m_RowsDirty = m_PreviewDirty = true;
    return true;
}
bool CCharacterActionWorkbench::Sound_SourceMs(const ROW& row, const int deltaMs, std::uint32_t& sourceMs) const
{
    const auto* binding = Selected_Binding();
    if (!binding || row.stage >= binding->Stages.size()) return false;
    float rate = 0.f;
    for (const auto& clip : binding->Stages[row.stage].Clips)
        if (clip.strClipName == row.clip) { rate = clip.fPlayRate; break; }
    if (!std::isfinite(rate) || rate <= 0.f) return false;
    const int moved = static_cast<int>(row.source) + static_cast<int>(std::lround(deltaMs * rate));
    sourceMs = static_cast<std::uint32_t>((std::max)(0, moved));
    return true;
}
bool CCharacterActionWorkbench::Apply_SoundEdit(const ROW& row, const std::uint32_t startMs, const std::string& eventName)
{
    if (!m_AnimationTool) { m_Status = "The Animation cue owner is unavailable."; return false; }
    if (!Target_IsCurrent()) { m_Status = "Restore the selected Character model before editing its Sound cues."; return false; }
    if (!m_AnimationTool->Apply_CharacterActionSoundEdit(m_Asset, row.clip, row.source, startMs, eventName, m_Status)) return false;
    Refresh_Cues();
    // The saved row identity carries the event and its source time, so follow it.
    const auto owner = row.id.find(".sound.");
    m_SelectedRow = owner == std::string::npos ? std::string{} :
        row.id.substr(0u, owner) + ".sound." + eventName + "." + std::to_string(startMs);
    m_SoundEditRow.clear();
    return true;
}
bool CCharacterActionWorkbench::Remove_Sound(const ROW& row)
{
    if (!m_AnimationTool) { m_Status = "The Animation cue owner is unavailable."; return false; }
    if (!Target_IsCurrent()) { m_Status = "Restore the selected Character model before editing its Sound cues."; return false; }
    if (!m_AnimationTool->Remove_CharacterActionSoundEvent(m_Asset, row.clip, row.source, row.asset, m_Status)) return false;
    Refresh_Cues();
    m_SelectedRow.clear(); m_SoundEditRow.clear();
    return true;
}
bool CCharacterActionWorkbench::Add_SoundAtPlayhead(const std::string& eventName)
{
    if (eventName.empty()) { m_Status = "Select a catalog sound event first."; return false; }
    if (!m_AnimationTool) { m_Status = "The Animation cue owner is unavailable."; return false; }
    if (!Target_IsCurrent()) { m_Status = "Restore the selected Character model before adding a Sound cue."; return false; }
    const std::uint32_t clock = Playhead_Ms();
    for (const auto& row : m_Rows)
    {
        if (row.kind != ROW_KIND::ANIMATION || clock < row.start || clock >= row.start + row.duration) continue;
        const auto* clip = Find_Clip(row.id);
        if (!clip || !std::isfinite(clip->fPlayRate) || clip->fPlayRate <= 0.f) break;
        const auto source = clip->iSourceStartMs +
            static_cast<std::uint32_t>(std::lround((clock - row.start) * clip->fPlayRate));
        if (!m_AnimationTool->Add_CharacterActionSoundEvent(m_Asset, row.clip, source, eventName, m_Status)) return false;
        Refresh_Cues();
        return true;
    }
    m_Status = "Move the playhead onto an Animation box before adding a Sound cue.";
    return false;
}
bool CCharacterActionWorkbench::Save_Bindings()
{
    if (!Target_IsCurrent()) { m_Status = "Restore the selected Character model before saving source windows."; return false; }
    for (const auto& binding : m_Bindings.Bindings)
        for (const auto& stage : binding.Stages)
            for (const auto& clip : stage.Clips)
            {
                std::uint32_t duration;
                if (!Clip_Duration(clip, duration)) { m_Status = "Save rejected invalid source window: " + clip.strClipName; return false; }
            }
    std::string committed;
    if (!CAnimationSkillBindingDocument::Save_AtomicWithBaseline(m_Bindings, m_Asset, CLASSES[m_ClassIndex].id,
        CPlayerSkillCatalog::Get_Skills(), m_ClipNames, m_Baseline, committed, m_Status)) return false;
    m_Baseline = std::move(committed); m_BindingsDirty = false;
    m_Status += " Preview uses this draft; re-enter the world to reload Product presentation.";
    return true;
}
bool CCharacterActionWorkbench::Save_Combat()
{
    if (!m_Combat.Save_Atomic(m_CombatRows, m_Status)) return false;
    m_CombatDirty = false; m_CombatPublishPending = true;
    m_Status += " Run Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish and restart Server to apply.";
    return true;
}
}
