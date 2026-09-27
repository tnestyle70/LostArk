#pragma once
#ifdef _DEBUG
#include "GuideAIDocument.h"
#include "Network/PacketMessages.h"
#include <optional>

namespace Client
{
class CGuideAITool final
{
public:
    void Open();
    void Update();
    void Render();
    bool Is_Open() const { return m_Open || m_CombatOpen; }
    bool Consume_InteractionRequest() { const bool result = m_InteractionRequested; m_InteractionRequested = false; return result; }
    void Set_RuntimeState(const LostArk::Shared::S2C_GUIDE_STATE* state) { if (state) m_Runtime = *state; else m_Runtime.reset(); }
    void Set_ActiveArea(std::string area) { m_ActiveArea = std::move(area); }
private:
    void Render_Prompts();
    void Render_Triggers();
    void Render_Combos();
    void Render_Combat();
    void Import_BernStart();
    void Refresh_References();
    void Render_BoxPreview(const DATA_JSON_VALUE& event);
    std::string New_Id(const char* prefix);
    CGuideAIDocument m_Document;
    bool m_Open = false, m_CombatOpen = false, m_InteractionRequested = false, m_Rewrite = false;
    bool m_ReferencesReady = false, m_ShowBox = true;
    std::string m_ActiveArea;
    std::string m_Category = "bern", m_PromptId, m_TriggerId, m_ComboId, m_CommandId;
    DATA_JSON_VALUE m_PromptEdit;
    DATA_JSON_VALUE m_NpcWorld;
    std::vector<std::pair<std::string,std::string>> m_NpcChoices, m_PatternChoices, m_SkillChoices;
    uint64_t m_NextId = 0;
    std::optional<LostArk::Shared::S2C_GUIDE_STATE> m_Runtime;
};
}
#endif
