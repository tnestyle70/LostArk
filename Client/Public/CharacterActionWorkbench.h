#pragma once

#include "AnimationEffectCueDocument.h"
#include "AnimationSkillBindingDocument.h"
#include "CharacterActionCombatDocument.h"
#include "CompositionResourceTree.h"
#include "CompositionWorkbenchSession.h"

#include <functional>
#include <memory>
#include <optional>

namespace Engine { class CCamera; }
namespace Client
{
class CAnimation_Tool;
class CCharacterPreviewPanel;
class CEffectAuthoringSequencer;

class CCharacterActionWorkbench final : public ICompositionWorkbenchSession
{
public:
    CCharacterActionWorkbench(std::shared_ptr<CCharacterPreviewPanel> panel,
        std::shared_ptr<CEffectAuthoringSequencer> sequencer, CAnimation_Tool* animationTool);
    ~CCharacterActionWorkbench() override;
    void Begin_WorkbenchFrame() override;
    void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) override;
    void End_WorkbenchFrame() override;
    void On_WorkbenchDeactivated() override;
    void On_LevelChanged();
    void Update(float dt, bool active);
    bool Consume_InteractionRequest();
    void Set_Camera(const std::shared_ptr<Engine::CCamera>& camera);
    void Set_OpenEffectResourceCallback(std::function<void(const std::string&)> callback);
    bool Can_AppendCompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
        bool replace, std::string& status) const override;
    bool Append_CompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
        bool replace, std::string& status) override;

private:
    enum class ROW_KIND { ANIMATION, EFFECT, SOUND, SHAKE, COLLIDER, LOGIC, RESULT };
    struct ROW final
    {
        ROW_KIND kind = ROW_KIND::ANIMATION;
        std::string id, label, clip, asset, detail;
        std::uint32_t start = 0u, duration = 1u, stage = 0u;
    };
    void Render_Actions();
    void Render_Timeline();
    void Render_Resources();
    void Render_Details();
    void Render_Transport();
    void Render_CombatDetail(const ROW& row);
    void Render_AnimationDetail(const ROW& row);
    void Render_CueEditor(const ROW& row);
    bool Select_Action(int classIndex, std::uint32_t skillId, std::optional<std::uint32_t> stage);
    bool Load_Class(int classIndex);
    bool Reload_Selected();
    bool Rebuild_Rows();
    bool Prepare_Preview();
    bool Restore_PreviewTarget();
    bool Save_Bindings();
    bool Save_Combat();
    bool Has_Draft() const;
    bool Target_IsCurrent() const;
    bool Clip_Duration(const ANIMATION_SKILL_CLIP& clip, std::uint32_t& duration) const;
    ANIMATION_SKILL_BINDING* Selected_Binding();
    const ANIMATION_SKILL_BINDING* Selected_Binding() const;
    ANIMATION_SKILL_CLIP* Find_Clip(const std::string& id);
    void Move_Clip(const std::string& id, int direction);
    void Apply_BoxGesture(const std::string& id, ROW_KIND kind, int gesture, int deltaMs);

    std::shared_ptr<CCharacterPreviewPanel> m_Panel;
    std::shared_ptr<CEffectAuthoringSequencer> m_Sequencer;
    CAnimation_Tool* m_AnimationTool = nullptr;
    std::function<void(const std::string&)> m_OpenEffect;
    ANIMATION_SKILL_BINDING_DOCUMENT m_Bindings;
    ANIMATION_EFFECT_CUE_DOCUMENT m_Cues;
    CCharacterActionCombatDocument m_Combat;
    std::vector<CHARACTER_ACTION_COMBAT_ROW> m_CombatRows;
    std::vector<std::string> m_ClipNames, m_EffectIds;
    std::vector<ROW> m_Rows;
    std::string m_Asset, m_Baseline, m_Status, m_CueStatus, m_SelectedRow;
    std::string m_SelectedResource, m_CueEditorClip;
    int m_ClassIndex = -1, m_ResourceKind = 0;
    std::uint32_t m_SkillId = 0u, m_Duration = 1u, m_PreviewClock = 0u;
    std::optional<std::uint32_t> m_Stage;
    std::uint64_t m_Generation = 0u;
    float m_Zoom = 120.f;
    bool m_BindingsDirty = false, m_CombatDirty = false, m_RowsDirty = false;
    bool m_PreviewDirty = true, m_Interaction = false, m_CatalogLoaded = false;
    bool m_SaveBindings = false, m_SaveCombat = false, m_Reload = false;
    bool m_EffectInventoryLoaded = false;
    char m_Search[160]{};
    std::string m_DragId;
    ROW_KIND m_DragKind = ROW_KIND::ANIMATION;
    int m_DragGesture = 0;
    float m_DragX = 0.f;
};
}
