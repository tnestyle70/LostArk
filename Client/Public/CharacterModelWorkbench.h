#pragma once
#include "AnimationSkillBindingDocument.h"
#include "CompositionWorkbenchSession.h"
#include "DataJson.h"
#include <filesystem>
#include <memory>

namespace Client
{
class CCharacterPreviewPanel;
class CEffectAuthoringSequencer;
// The interaction/vehicle catalogs retain Product ownership; this adapter only
// projects their exact clips into the existing model/Effect sequencer.
class CCharacterModelWorkbench final
{
public:
    CCharacterModelWorkbench(std::shared_ptr<CCharacterPreviewPanel> panel,
        std::shared_ptr<CEffectAuthoringSequencer> sequencer);
    bool Render_Actions(bool locked);
    void Render(COMPOSITION_WORKBENCH_PANE pane);
    bool Is_Dirty() const;
private:
    struct ACTION final
    {
        std::string id, category, label, asset, mode;
        uint32_t vehicleId = 0u, skillId = 0u, modeSlot = 0u;
        bool locomotion = false, sourceAction = false, lifetime = false;
    };
    bool Refresh();
    bool Select(const ACTION& action);
    bool Save_Product();
    const DATA_JSON_VALUE* Find_Action(const DATA_JSON_VALUE& root, const ACTION& action) const;
    bool Replace_Action(DATA_JSON_VALUE& root, const ACTION& action, DATA_JSON_VALUE replacement) const;
    std::filesystem::path Owner_Path(const ACTION& action) const;
    std::shared_ptr<CCharacterPreviewPanel> m_Panel;
    std::shared_ptr<CEffectAuthoringSequencer> m_Sequencer;
    std::vector<ACTION> m_Actions;
    ACTION m_Selected;
    DATA_JSON_VALUE m_BaselineAction;
    std::string m_Status;
    bool m_Loaded = false;
};
}
