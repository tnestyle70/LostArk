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
#include "EffectAuthoringResourceTree.h"
#include "EffectAuthoringSequencer.h"
#include "EffectEditingSession.h"
#include "Model.h"
#include "PlayerSkillCatalog.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <set>

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
    if (m_Panel) m_Panel->Refresh_Level();
    if (m_RowsDirty && Target_IsCurrent()) Rebuild_Rows();
}
void CCharacterActionWorkbench::End_WorkbenchFrame()
{
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
    m_Asset = entry.asset; m_ClassIndex = classIndex; m_Generation = CAnimationTargetService::Resolve_TargetGeneration();
    m_BindingsDirty = m_CombatDirty = false; m_PreviewDirty = true; m_SelectedRow.clear();
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
    std::vector<ROW> rows;
    std::uint32_t clock = 0u, end = 1u;
    for (std::size_t stage = 0u; stage < binding->Stages.size(); ++stage)
    {
        if (m_Stage && *m_Stage != stage) continue;
        const std::uint32_t stageStart = clock;
        for (const auto& clip : binding->Stages[stage].Clips)
        {
            std::uint32_t duration = 0u;
            if (!Clip_Duration(clip, duration)) { m_Status = "Invalid source window or missing clip: " + clip.strClipName; return false; }
            const auto stageId = static_cast<std::uint32_t>(stage);
            rows.push_back({ROW_KIND::ANIMATION, clip.strClipOccurrenceId, "Animation | " + clip.strClipName,
                clip.strClipName, {}, "skillbindings.json", clock, duration, stageId});
            const std::uint32_t sourceEnd = clip.iSourceStartMs + static_cast<std::uint32_t>(std::ceil(duration * clip.fPlayRate));
            const auto within = [&](std::uint32_t time) { return time >= clip.iSourceStartMs && time < sourceEnd; };
            const auto local = [&](std::uint32_t time) { return static_cast<std::uint32_t>((time - clip.iSourceStartMs) / clip.fPlayRate); };
            for (const auto& cue : m_Cues.Cues)
            {
                if (cue.strClipName != clip.strClipName || !within(cue.iStartMs)) continue;
                const auto length = cue.iEndMs > cue.iStartMs ? static_cast<std::uint32_t>((cue.iEndMs - cue.iStartMs) / clip.fPlayRate) : 100u;
                rows.push_back({ROW_KIND::EFFECT, clip.strClipOccurrenceId + ".effect." + cue.strEffectAssetId + "." + std::to_string(cue.iStartMs),
                    "Effect | " + cue.strEffectAssetId, clip.strClipName, cue.strEffectAssetId, "Cue owner: .animevents; anchor: " + cue.strAnchorSlotId,
                    clock + local(cue.iStartMs), (std::max)(1u, length), stageId});
            }
            for (const auto& cue : m_Cues.Sounds)
                if (cue.strClipName == clip.strClipName && within(cue.iStartMs))
                    rows.push_back({ROW_KIND::SOUND, clip.strClipOccurrenceId + ".sound." + cue.strEventName + "." + std::to_string(cue.iStartMs),
                        "Sound | " + cue.strEventName, clip.strClipName, {}, "Cue owner: .animevents / variants: CharacterSoundCatalog.json",
                        clock + local(cue.iStartMs), 80u, stageId});
            for (const auto& cue : m_Cues.Shakes)
                if (cue.strClipName == clip.strClipName && within(cue.iStartMs))
                    rows.push_back({ROW_KIND::SHAKE, clip.strClipOccurrenceId + ".shake." + std::to_string(cue.iStartMs),
                        "Camera Shake", clip.strClipName, {}, "Cue owner: .animevents", clock + local(cue.iStartMs), 80u, stageId});
            clock += duration;
        }
        for (const auto& collider : m_CombatRows)
        {
            if (collider.iSkillId != m_SkillId || collider.iStageIndex != stage) continue;
            const auto start = stageStart + collider.iProjectileStartMs + collider.iTimeMs;
            const auto duration = (std::max)(34u, collider.iRepeatMs * (collider.iRepeatCount > 0u ? collider.iRepeatCount - 1u : 0u) + 34u);
            const auto owner = collider.iProjectileIndex == UINT32_MAX ? "Caster" : "Projectile " + std::to_string(collider.iProjectileIndex + 1u);
            const auto detail = owner + (collider.bContact ? " | CONTACT: dynamic overlap" : " | TIMED") + " | " +
                collider.strColliderId + " -> " + collider.strLogicId + " -> " + collider.strResultId;
            rows.push_back({ROW_KIND::COLLIDER, collider.strColliderId, "Collider | " + collider.strResultKind, {}, {}, detail, start, duration, static_cast<std::uint32_t>(stage)});
            rows.push_back({ROW_KIND::LOGIC, collider.strLogicId, "Logic | AREA_OVERLAP", {}, {}, detail, start, duration, static_cast<std::uint32_t>(stage)});
            rows.push_back({ROW_KIND::RESULT, collider.strResultId, "Result | " + collider.strResultKind, {}, {}, detail, start, duration, static_cast<std::uint32_t>(stage)});
        }
    }
    for (const auto& row : rows) end = (std::max)(end, row.start + row.duration);
    m_Rows = std::move(rows); m_Duration = end; m_RowsDirty = false;
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
}

void CCharacterActionWorkbench::Render_Timeline()
{
    if (!Selected_Binding()) { ImGui::TextWrapped("Select a Character action or its Parent to see Animation, Effect, Sound and Collider / Logic / Result rows."); return; }
    Render_Transport();
    if (m_Sequencer && !m_PreviewDirty) m_Duration = (std::max)(m_Duration, m_Sequencer->Preview_DurationMs());
    int clock = static_cast<int>(m_PreviewDirty ? m_PreviewClock : m_Sequencer ? m_Sequencer->ClockMs() : 0u);
    ImGui::SetNextItemWidth(280.f);
    if (ImGui::SliderInt("Time##Character", &clock, 0, static_cast<int>(m_Duration), "%d ms") && Prepare_Preview())
        m_Sequencer->Seek(static_cast<std::uint32_t>(clock));
    ImGui::SameLine(); ImGui::SetNextItemWidth(150.f); ImGui::SliderFloat("Zoom##Character", &m_Zoom, 20.f, 400.f, "%.0f px/s");
    ImGui::TextDisabled("Animation boxes: reorder by dragging; edges trim source. Collider boxes move their own judgement time.");
    if (ImGui::BeginChild("CharacterActionTimeline", {0.f, 0.f}, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
    {
        constexpr float labelWidth = 300.f, rowHeight = 28.f;
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = (std::max)(ImGui::GetContentRegionAvail().x, labelWidth + m_Duration * m_Zoom * .001f + 80.f);
        auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, {origin.x + labelWidth, origin.y}, {origin.x + width, origin.y + rowHeight}, m_Duration, m_Zoom);
        ImGui::Dummy({width, rowHeight});
        for (const auto& row : m_Rows)
        {
            ImGui::PushID(row.id.c_str());
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const bool selected = m_SelectedRow == row.id;
            if (ImGui::Selectable(row.label.c_str(), selected, 0, {labelWidth - 8.f, rowHeight - 3.f}))
            { m_SelectedRow = row.id; if (m_Sequencer) m_Sequencer->Pause(true); }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s", row.id.c_str(), row.detail.c_str());
            const float startX = p.x + labelWidth + row.start * m_Zoom * .001f;
            const float endX = startX + (std::max)(8.f, row.duration * m_Zoom * .001f);
            const bool editable = row.kind == ROW_KIND::ANIMATION || row.kind == ROW_KIND::COLLIDER;
            const ImU32 colors[] = {IM_COL32(69,126,203,255), IM_COL32(70,166,111,255), IM_COL32(175,127,60,255), IM_COL32(167,102,174,255), IM_COL32(200,96,76,255), IM_COL32(122,121,175,255), IM_COL32(170,102,77,255)};
            CompositionTimeline::DrawBox(draw, {startX, p.y + 1.f}, {endX, p.y + rowHeight - 4.f},
                colors[static_cast<std::size_t>(row.kind)], selected, row.label.c_str(), row.kind == ROW_KIND::ANIMATION, row.kind == ROW_KIND::ANIMATION);
            const ImVec2 mouse = ImGui::GetMousePos();
            const bool hover = mouse.x >= startX && mouse.x <= endX && mouse.y >= p.y && mouse.y <= p.y + rowHeight;
            if (hover && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                m_SelectedRow = row.id;
                if (m_Sequencer) m_Sequencer->Pause(true);
                if (editable)
                {
                    m_DragId = row.id; m_DragKind = row.kind; m_DragX = mouse.x;
                    m_DragGesture = static_cast<int>(CompositionTimeline::HitBoxGesture(mouse.x, startX, endX, 5.f,
                        row.kind == ROW_KIND::ANIMATION, row.kind == ROW_KIND::ANIMATION));
                }
            }
            ImGui::SetCursorScreenPos({p.x, p.y + rowHeight});
            ImGui::PopID();
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
        found->iTimeMs = static_cast<std::uint32_t>((std::clamp)(static_cast<int>(found->iTimeMs) + deltaMs, 0, 600000));
        m_CombatDirty = m_RowsDirty = m_PreviewDirty = true;
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
    if (found == m_Rows.end()) { ImGui::TextDisabled("Select an Animation, Effect, Collider, Logic or Result box."); return; }
    const ROW row = *found;
    ImGui::TextWrapped("%s", row.label.c_str());
    ImGui::TextWrapped("%s", row.id.c_str());
    ImGui::Text("Stage %u | %u + %u ms", row.stage + 1u, row.start, row.duration);
    ImGui::TextWrapped("%s", row.detail.c_str());
    if (row.kind == ROW_KIND::ANIMATION) Render_AnimationDetail(row);
    else if (row.kind == ROW_KIND::COLLIDER || row.kind == ROW_KIND::LOGIC || row.kind == ROW_KIND::RESULT) Render_CombatDetail(row);
    else
    {
        if (!row.asset.empty() && m_OpenEffect && ImGui::Button("Edit Effect resource")) m_OpenEffect(row.asset);
        ImGui::SeparatorText("Animation cue owner");
        ImGui::TextWrapped("Edit source cue timing, anchor and transform below; Save writes the same Product .animevents owner. Refresh action cues after saving.");
        if (ImGui::Button("Refresh saved cues"))
        {
            ANIMATION_EFFECT_CUE_DOCUMENT staged;
            if (CAnimationEffectCueDocument::Load(m_Asset, m_ClipNames, staged, m_Status))
            { m_Cues = std::move(staged); m_RowsDirty = m_PreviewDirty = true; }
        }
        Render_CueEditor(row);
    }
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
    else ImGui::TextUnformatted("Owner: Caster");
    if (value.bContact) ImGui::TextWrapped("CONTACT has no fixed hit time. Its box marks the projectile start; the Server triggers the result when contact occurs.");
    changed |= Edit_U32("Repeat count", value.iRepeatCount, 256);
    changed |= Edit_U32("Repeat interval (ms)", value.iRepeatMs);
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
    ImGui::TextWrapped("Save updates HitShapes authoring. Publish gameplay and restart Server to activate the saved judgement changes.");
}

void CCharacterActionWorkbench::Render_Resources()
{
    const char* tabs[] = {"Animation", "Effect", "Collider / Result"};
    for (int i = 0; i < 3; ++i)
    { if (i) ImGui::SameLine(); if (ImGui::Selectable(tabs[i], m_ResourceKind == i, 0, {140.f, 0.f})) m_ResourceKind = i; }
    ImGui::Separator();
    if (m_ResourceKind == 0)
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
        return;
    }
    if (m_ResourceKind == 1)
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
        return;
    }
    for (const auto& value : m_CombatRows)
        if (value.iSkillId == m_SkillId)
        {
            const std::string label = value.strResultKind + " | " + value.strColliderId;
            if (ImGui::Selectable(label.c_str(), m_SelectedRow == value.strColliderId)) m_SelectedRow = value.strColliderId;
        }
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
    m_CombatDirty = false;
    m_Status += " Publish gameplay and restart Server to apply.";
    return true;
}
}
