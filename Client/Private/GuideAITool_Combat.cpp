#include "imgui.h"
#include "GuideAITool.h"
#ifdef _DEBUG
namespace Client
{
using namespace GuideJson;
void CGuideAITool::Render_Combat()
{
    if(!m_CombatOpen)return;
    ImGui::SetNextWindowSize(ImVec2(670,660),ImGuiCond_FirstUseEver);
    if(ImGui::Begin("Guide AI - Combat Detail",&m_CombatOpen))
    {
        m_InteractionRequested|=ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        ImGui::TextWrapped("Choose the highest action score: Follow = wF * clamp(distance error / resume), outside the following band; Evade = wE * clamp(threat); Combat = wA when help and a target are active. Survival override chooses evade first.");
        ImGui::BeginDisabled(m_Document.Is_Busy()||m_Rewrite);
        auto combat=Field(m_Document.Draft(),"combat"),weights=Field(combat,"weights");
        if(ImGui::BeginTable("Weights",4,ImGuiTableFlags_Borders)){ImGui::TableSetupColumn("Situation");ImGui::TableSetupColumn("Attack");ImGui::TableSetupColumn("Avoid");ImGui::TableSetupColumn("Follow");ImGui::TableHeadersRow();for(const auto* mode:{"FOLLOW","ASSIST"}){auto row=Field(weights,mode);ImGui::PushID(mode);ImGui::TableNextRow();ImGui::TableNextColumn();ImGui::TextUnformatted(mode);for(const auto* key:{"attack","avoid","follow"}){ImGui::TableNextColumn();float value=static_cast<float>(Number(row,key));ImGui::SetNextItemWidth(-1);ImGui::BeginDisabled(std::string(mode)=="FOLLOW"&&std::string(key)=="attack");if(ImGui::DragFloat(key,&value,.01f,0.f,1.f,"%.2f"))Set(row,key,J::Number(value,true));ImGui::EndDisabled();}Set(weights,mode,std::move(row));ImGui::PopID();}ImGui::EndTable();}
        ImGui::TextUnformatted("Each row must sum to 1. Save validates the complete policy.");Set(combat,"weights",std::move(weights));
        const auto edit=[&](const char* section,const std::vector<std::pair<const char*,const char*>>& fields){auto value=Field(combat,section);if(ImGui::CollapsingHeader(section,ImGuiTreeNodeFlags_DefaultOpen)){for(const auto& [key,label]:fields){float n=static_cast<float>(Number(value,key));if(ImGui::DragFloat(label,&n,std::string(key).find("Ms")!=std::string::npos?10.f:.05f))Set(value,key,J::Number(n,true));}}Set(combat,section,std::move(value));};
        edit("follow",{{"desiredDistanceM","Desired distance (m)"},{"minimumDistanceM","Minimum distance (m)"},{"maximumDistanceM","Maximum distance (m)"},{"resumeDistanceM","Resume follow distance (m)"},{"recoverDistanceM","Recovery distance (m)"},{"recoverDelayMs","Recovery delay (ms)"}});
        edit("decision",{{"thinkIntervalMs","Decision interval (ms)"},{"horizonMs","Known hazard horizon (ms)"},{"switchMargin","Switch score margin"},{"minimumHoldMs","Minimum decision hold (ms)"},{"lethalHpFraction","Lethal HP threshold"}});
        Set(m_Document.Draft(),"combat",std::move(combat));ImGui::EndDisabled();
        ImGui::Separator();ImGui::TextUnformatted("Server decision (read only)");
        if(m_Runtime)
        {
            const auto& state=*m_Runtime;
            const char* actions[]={"Idle","Follow","Evade","Combat","Recover","Contact reaction","Vehicle follow","Human party down","Wait outside minigame"};
            ImGui::Text("Guide %u / anchor %u / tick %u",state.iGuideNetEntityId,state.iOwnerNetEntityId,state.iServerTick);
            ImGui::Text("Applied revision %llu / %s / %s",static_cast<unsigned long long>(state.iRevision),state.iContext==0?"Follow / explain":"Combat assistance",state.iAction<9?actions[state.iAction]:"Unknown");
            ImGui::Text("Follow %.3f / evade %.3f / combat %.3f",state.fFollowScore,state.fEvadeScore,state.fCombatScore);
            ImGui::Text("Threat %.3f / anchor %.2f m / HP %.1f%%",state.fThreat,state.fAnchorDistance,state.fHpRatio*100.f);
            ImGui::Text("Survival override: %s",state.bSurvivalOverride?"Active":"Inactive");
            ImGui::TextWrapped("Reason: %s",state.strReason.c_str());
            ImGui::TextWrapped("Combo: %s / admitted steps %u",state.strComboId.c_str(),state.iComboStep);
        }
        else ImGui::TextWrapped("No Guide state received from the active Server. Draft weights are not live simulation results.");
    }
    ImGui::End();
}
}
#endif
