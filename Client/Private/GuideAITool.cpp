#include "imgui.h"
#include "GuideAITool.h"

//왜 여기에 debug가 달려있는 거지? release에서도 사용해야 하는 Guide 관련된 툴인 거 아닌가?
#ifdef _DEBUG
#include "ProjectDataRoot.h"
#include "GameInstance.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <sstream>

namespace Client
{
using namespace GuideJson;
//ananymous namespace
namespace
{

bool EditText(const char* label, J& value, const char* key, bool multiline = false)
{
    std::array<char, 8192> buffer{}; const auto text = String(value,key); std::copy_n(text.data(), (std::min)(text.size(), buffer.size()-1), buffer.data());
    const bool changed = multiline ? ImGui::InputTextMultiline(label,buffer.data(),buffer.size(),ImVec2(-1,110)) : ImGui::InputText(label,buffer.data(),buffer.size());
    if (changed) Set(value,key,J::String(buffer.data())); return changed;
}

bool EditNumber(const char* label, J& value, const char* key, float speed = 0.1f)
{ float number = static_cast<float>(Number(value,key)); if (!ImGui::DragFloat(label,&number,speed)) return false; Set(value,key,J::Number(number,true)); return true; }

bool EditMilliseconds(const char* label, J& value, const char* key)
{ int number = static_cast<int>(Number(value,key)); if (!ImGui::InputInt(label,&number)) return false; Set(value,key,J::Number(number)); return true; }

bool EditBool(const char* label, J& value, const char* key)
{ bool flag=Boolean(value,key); if(!ImGui::Checkbox(label,&flag))return false;Set(value,key,J::Boolean(flag));return true; }
//edit vector, pos, rot, scale을 조정하기 위한 API?
bool EditVector(const char* label, J& value, const char* key)
{ float xyz[3]{}; const auto& source=Field(value,key).Get_Array();
    for(size_t i=0;i<(std::min)(source.size(),size_t(3));++i)
        xyz[i]=static_cast<float>(source[i].Get_Number());
    if(!ImGui::DragFloat3(label,xyz,0.1f)) return false;
    Set(value,key,J::Array({J::Number(xyz[0],true),J::Number(xyz[1],true),J::Number(xyz[2],true)}));
    return true;
}

bool EditBoxVector(const char* label, J& event, const char* key, double displayScale, double minimum, double maximum)
{
    const auto& source = Field(event, key).Get_Array();
    if (source.size() != 3) return false;
    float xyz[3];
    for (size_t i = 0; i < 3; ++i) xyz[i] = static_cast<float>(source[i].Get_Number() * displayScale);
    if (!ImGui::DragFloat3(label, xyz, 0.1f, static_cast<float>(minimum), static_cast<float>(maximum), "%.3f", ImGuiSliderFlags_AlwaysClamp)) return false;
    for (const float component : xyz) if (!std::isfinite(component)) return false;
    // Clamp in the JSON number domain: float(0.02) / 2 is slightly below the schema's 0.01 minimum.
    const auto stored = [&](float component) { return std::clamp(component / displayScale, minimum / displayScale, maximum / displayScale); };
    Set(event, key, J::Array({J::Number(stored(xyz[0]), true), J::Number(stored(xyz[1]), true), J::Number(stored(xyz[2]), true)}));
    return true;
}

bool Choose(const char* label,J& value,const char* key,const std::vector<std::pair<std::string,std::string>>& choices)
{ bool changed=false;auto current=String(value,key);if(ImGui::BeginCombo(label,current.c_str())){for(const auto& [id,name]:choices)if(ImGui::Selectable(name.c_str(),id==current)){Set(value,key,J::String(id));changed=true;}ImGui::EndCombo();}return changed; }

std::vector<std::pair<std::string,std::string>> Choices(const J::ARRAY& rows,const char* id,const char* label,bool empty=false)
{std::vector<std::pair<std::string,std::string>> result;if(empty)result.emplace_back("","None");for(const auto& row:rows)result.emplace_back(String(row,id),String(row,label));return result;}
size_t Selected(const J::ARRAY& rows,const char* key,const std::string& id)
{for(size_t i=0;i<rows.size();++i)if(String(rows[i],key)==id)return i;return rows.size();}
J Str(const char* value){return J::String(value);}
bool References(const J& root,const char* field,const std::string& id)
{for(const auto& row:Field(Field(root,"triggers"),"triggers").Get_Array())if(String(row,field)==id)return true;if(std::string(field)=="comboId")for(const auto& row:Field(Field(root,"combat"),"commands").Get_Array())if(String(row,field)==id)return true;return false;}
}
std::string CGuideAITool::New_Id(const char* prefix){return std::string(prefix)+"."+std::to_string(GetTickCount64())+"."+std::to_string(++m_NextId);}
void CGuideAITool::Open(){m_Open=true;if(!m_Document.Is_Loaded())m_Document.Load();}
void CGuideAITool::Update(){m_Document.Poll();if(m_Document.Is_Loaded()&&!m_Document.Is_Busy()&&!m_ReferencesReady)Refresh_References();}
bool CGuideAITool::Is_PlacementPickArmed() const
{
    if (!m_ColliderOpen || !m_Document.Is_Loaded() || m_Document.Is_Busy() || m_Rewrite ||
        m_PickTriggerId.empty() || m_TriggerId != m_PickTriggerId || m_Category != m_PickCategory)
        return false;
    const auto& rows = Field(Field(m_Document.Draft(), "triggers"), "triggers").Get_Array();
    const auto index = Selected(rows, "triggerId", m_PickTriggerId);
    if (index >= rows.size()) return false;
    const auto& event = Field(rows[index], "event");
    return String(rows[index], "categoryId") == m_PickCategory &&
        String(event, "type") == "SPACE_ENTER" && String(event, "boxId") == m_PickBoxId &&
        CGuideAIDocument::Serialize(event) == m_PickEvent;
}
bool CGuideAITool::Consume_PlacementPickRequest()
{
    const bool requested = m_PickRequested;
    m_PickRequested = false;
    return requested;
}
void CGuideAITool::Cancel_PlacementPick(std::string reason)
{
    m_PickRequested = false;
    m_PickArea.clear(); m_PickCategory.clear(); m_PickTriggerId.clear(); m_PickBoxId.clear(); m_PickEvent.clear();
    if (!reason.empty()) m_Document.Set_Status(std::move(reason));
}
void CGuideAITool::Complete_PlacementPick(const float3_t& position)
{
    if (!Is_PlacementPickArmed() || !std::isfinite(position.x) ||
        !std::isfinite(position.y) || !std::isfinite(position.z))
    { Cancel_PlacementPick("The selected Guide box changed; its previous position was preserved."); return; }
    auto document = Field(m_Document.Draft(), "triggers");
    auto rows = Field(document, "triggers").Get_Array();
    const auto index = Selected(rows, "triggerId", m_PickTriggerId);
    auto event = Field(rows[index], "event");
    Set(event, "position", J::Array({J::Number(position.x, true), J::Number(position.y, true), J::Number(position.z, true)}));
    Set(event, "anchorPlacementId", Str(""));
    Set(rows[index], "event", std::move(event));
    Set(document, "triggers", J::Array(std::move(rows)));
    Set(m_Document.Draft(), "triggers", std::move(document));
    Cancel_PlacementPick("Picked position staged in this Guide box. Save keeps the source; Publish updates runtime files.");
}
void CGuideAITool::Refresh_References()
{
    m_ReferencesReady=true;
    const auto selectCategoryRow=[&](const J::ARRAY& rows,const char* key,std::string& selected){const auto found=Selected(rows,key,selected);if(found<rows.size()&&String(rows[found],"categoryId")==m_Category)return;selected.clear();for(const auto& row:rows)if(String(row,"categoryId")==m_Category){selected=String(row,key);break;}};
    selectCategoryRow(Field(Field(m_Document.Draft(),"prompts"),"prompts").Get_Array(),"promptId",m_PromptId);
    selectCategoryRow(Field(Field(m_Document.Draft(),"triggers"),"triggers").Get_Array(),"triggerId",m_TriggerId);
    const auto& combos=Field(Field(m_Document.Draft(),"combat"),"combos").Get_Array();if(m_ComboId.empty()&&!combos.empty())m_ComboId=String(combos.front(),"comboId");
    const auto& commands=Field(Field(m_Document.Draft(),"combat"),"commands").Get_Array();if(m_CommandId.empty()&&!commands.empty())m_CommandId=String(commands.front(),"commandId");
    m_NpcChoices={{"","No NPC anchor"}};m_PatternChoices.clear();m_SkillChoices.clear();m_NpcWorld=J::Null();
    std::string area,status;for(const auto& c:Field(Field(m_Document.Draft(),"catalog"),"categories").Get_Array())if(String(c,"categoryId")==m_Category)area=String(c,"areaId");
    if(!area.empty()&&CGuideAIDocument::Read(CProjectDataRoot::Resolve(std::filesystem::path("Worlds")/area/"Gameplay.world.json"),m_NpcWorld,status))for(const auto& p:Field(m_NpcWorld,"placements").Get_Array())if(String(p,"kind")=="npc")m_NpcChoices.emplace_back(String(p,"placementId"),String(p,"placementId"));
    std::ifstream input(CProjectDataRoot::Get().parent_path()/"Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap");std::string line;while(std::getline(input,line))if(line.rfind("PATTERN\t",0)==0){std::istringstream cells(line);std::string tag,encounter,id;std::getline(cells,tag,'\t');std::getline(cells,encounter,'\t');std::getline(cells,id,'\t');if((m_Category=="valtan"&&encounter.find("VALTAN")!=std::string::npos)||(m_Category=="koukusaydon"&&encounter.find("KAKUL")!=std::string::npos))m_PatternChoices.emplace_back(id,id);}
    J skillDoc;if(CGuideAIDocument::Read(CProjectDataRoot::Resolve("Balance/PlayerSkills.json"),skillDoc,status))for(const auto& s:Field(skillDoc,"skills").Get_Array())if(String(s,"characterClass")=="DIMENSIONMASTER"&&String(s,"skillKind")=="ACTIVE")m_SkillChoices.emplace_back(String(s,"inputSlot"),String(s,"inputSlot")+" / "+String(s,"displayName")+" / "+std::to_string(static_cast<uint32_t>(Number(s,"skillId"))));
}
void CGuideAITool::Import_BernStart()
{
    J world;std::string status;if(!CGuideAIDocument::Read(CProjectDataRoot::Resolve("Worlds/LV_BER_BERNCASTLE/Gameplay.world.json"),world,status)){m_Document.Set_Status(status);return;}
    for(const auto& row:Field(world,"placements").Get_Array())if(String(row,"placementId")=="player_1")
    {auto placement=Field(m_Document.Draft(),"placement");Set(placement,"position",Field(row,"position"));Set(placement,"yawDegrees",Field(row,"yawDegrees"));Set(m_Document.Draft(),"placement",std::move(placement));m_Document.Set_Status("Copied current Bern player_1 transform into the Guide draft.");return;}
    m_Document.Set_Status("Bern player_1 is missing; draft preserved.");
}
void CGuideAITool::Render()
{
    if(!m_Open)
    {
        Render_ColliderDetail();
        if(m_ColliderOpen && m_Document.Is_Loaded())Render_DebugBoxes();
        Render_Combat();return;
    }
    ImGui::SetNextWindowSize(ImVec2(960,860),ImGuiCond_FirstUseEver);
    if(ImGui::Begin("DimensionMaster Guide",&m_Open))
    {
        m_InteractionRequested|=ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        ImGui::BeginDisabled(m_Document.Is_Busy()||m_Rewrite);
        if(ImGui::Button("Load")){Cancel_PlacementPick({});if(m_Document.Load())m_ReferencesReady=false;}ImGui::SameLine();if(ImGui::Button("Save")){Cancel_PlacementPick({});m_Document.Start_Save();}ImGui::SameLine();if(ImGui::Button("Publish")){Cancel_PlacementPick({});m_Document.Start_Publish();}
        ImGui::SameLine();ImGui::TextUnformatted(m_Document.Is_Dirty()?"Modified source":"Saved source");ImGui::EndDisabled();
        ImGui::TextWrapped("%s",m_Document.Status().c_str());
        if(m_Document.Is_Loaded())
        {
            ImGui::TextUnformatted("가이드 차원술사 / DIMENSIONMASTER");
            ImGui::Text("Published revision: %llu / Server revision: %llu",static_cast<unsigned long long>(m_Document.Published_Revision()),static_cast<unsigned long long>(m_Runtime?m_Runtime->iRevision:0));
            ImGui::Checkbox("Show Debug##GuideColliders", &m_ShowBox);
            ImGui::SameLine();ImGui::Checkbox("All colliders in active Area", &m_ShowAllBoxes);
            if(m_ShowBox)ImGui::TextWrapped("Draft preview: yellow = selected, cyan = enabled, gray = disabled. Save / Publish are separate.");
            ImGui::BeginDisabled(m_Document.Is_Busy()||m_Rewrite);
            auto placement=Field(m_Document.Draft(),"placement");EditVector("Start position (m)",placement,"position");EditNumber("Rotation Y (degrees)",placement,"yawDegrees",0.5f);
            ImGui::TextUnformatted("One server Guide follows one player at a time and waits in Bern during travel.");Set(m_Document.Draft(),"placement",std::move(placement));
            if(ImGui::Button("베른 시작 위치 가져오기"))Import_BernStart();
            ImGui::Separator();for(const auto& category:Field(Field(m_Document.Draft(),"catalog"),"categories").Get_Array())
            {const auto id=String(category,"categoryId");if(ImGui::Selectable(String(category,"displayName").c_str(),m_Category==id,0,ImVec2(140,0))){m_Category=id;Refresh_References();}ImGui::SameLine();}ImGui::NewLine();ImGui::EndDisabled();
            ImGui::BeginDisabled(m_Document.Is_Busy());
            if(ImGui::BeginTabBar("GuideSections"))
            {
                if(ImGui::BeginTabItem("대사")){Render_Prompts();ImGui::EndTabItem();}
                if(ImGui::BeginTabItem("콜라이더")){ImGui::BeginDisabled(m_Rewrite);Render_Triggers(true);ImGui::EndDisabled();ImGui::EndTabItem();}
                if(ImGui::BeginTabItem("트리거")){ImGui::BeginDisabled(m_Rewrite);Render_Triggers();ImGui::EndDisabled();ImGui::EndTabItem();}
                if(ImGui::BeginTabItem("콤보 / 도움 명령")){ImGui::BeginDisabled(m_Rewrite);Render_Combos();ImGui::EndDisabled();ImGui::EndTabItem();}
                ImGui::EndTabBar();
            }
            ImGui::EndDisabled();ImGui::Separator();if(ImGui::Button("Combat Detail"))m_CombatOpen=true;
        }
    }
    ImGui::End();
    Render_ColliderDetail();
    if((m_Open || m_ColliderOpen) && m_Document.Is_Loaded())Render_DebugBoxes();
    Render_Combat();
}
void CGuideAITool::Render_Prompts()
{
    auto document=Field(m_Document.Draft(),"prompts");auto rows=Field(document,"prompts").Get_Array();
    ImGui::BeginChild("PromptList",ImVec2(230,390),true);ImGui::BeginDisabled(m_Rewrite);
    for(const auto& row:rows)if(String(row,"categoryId")==m_Category){const auto id=String(row,"promptId");ImGui::PushID(id.c_str());if(ImGui::Selectable(String(row,"title").c_str(),m_PromptId==id))m_PromptId=id;ImGui::PopID();}
    if(ImGui::Button("+ Row")){m_PromptId=New_Id("guide.prompt");rows.push_back(J::Object({{"promptId",J::String(m_PromptId)},{"categoryId",J::String(m_Category)},{"topic",Str("NPC")},{"title",Str("새 대사")},{"segments",J::Array({J::Object({{"text",Str("새 안내 문구를 입력하세요.")},{"durationMs",J::Number(6000)}})})}}));}
    ImGui::EndDisabled();ImGui::EndChild();ImGui::SameLine();ImGui::BeginChild("PromptDetails",ImVec2(0,390),true);
    const auto index=Selected(rows,"promptId",m_PromptId);
    if(index<rows.size())
    {
        auto& row=rows[index];ImGui::TextWrapped("%s",m_PromptId.c_str());
        if(!m_Rewrite)
        {ImGui::TextUnformatted(String(row,"title").c_str());for(const auto& segment:Field(row,"segments").Get_Array())ImGui::TextWrapped("%s",String(segment,"text").c_str());if(ImGui::Button("Rewrite")){m_PromptEdit=row;m_Rewrite=true;}ImGui::SameLine();if(ImGui::Button("Delete")){if(References(m_Document.Draft(),"promptId",m_PromptId))m_Document.Set_Status("Prompt is referenced by a trigger. Change its binding first.");else{rows.erase(rows.begin()+index);m_PromptId.clear();}}}
        else
        {
            EditText("Title",m_PromptEdit,"title");Choose("Topic",m_PromptEdit,"topic",{{"PARTY","Party"},{"NPC","NPC"},{"BOSS_PATTERN","Boss Pattern"},{"COMBAT","Combat"}});
            auto segments=Field(m_PromptEdit,"segments").Get_Array();for(size_t i=0;i<segments.size();++i){ImGui::PushID(static_cast<int>(i));EditText("Text",segments[i],"text",true);EditMilliseconds("Duration (ms)",segments[i],"durationMs");if(segments.size()>1&&ImGui::SmallButton("Remove segment")){segments.erase(segments.begin()+i);ImGui::PopID();break;}ImGui::PopID();}
            if(segments.size()<16&&ImGui::Button("+ Segment"))segments.push_back(J::Object({{"text",Str("새 안내 문구")},{"durationMs",J::Number(6000)}}));Set(m_PromptEdit,"segments",J::Array(std::move(segments)));
            if(ImGui::Button("Apply")){bool valid=true;size_t bytes=0;double duration=0;for(const auto& s:Field(m_PromptEdit,"segments").Get_Array()){const auto text=String(s,"text");const auto ms=Number(s,"durationMs");valid&=!text.empty()&&text.size()<=512&&ms>=1000&&ms<=20000;bytes+=text.size();duration+=ms;}if(valid&&bytes<=4096&&duration<=60000&&!String(m_PromptEdit,"title").empty()){row=m_PromptEdit;m_Rewrite=false;}else m_Document.Set_Status("Each segment needs 1..512 UTF-8 bytes and 1..20 seconds; total <=4096 bytes / 60 seconds.");}ImGui::SameLine();if(ImGui::Button("Cancel")){m_Rewrite=false;m_PromptEdit=J::Null();}
        }
    }
    else ImGui::TextUnformatted("대사를 선택하거나 + Row로 추가하세요.");
    ImGui::EndChild();Set(document,"prompts",J::Array(std::move(rows)));Set(m_Document.Draft(),"prompts",std::move(document));
}
void CGuideAITool::Render_Triggers(bool collidersOnly)
{
    auto document = Field(m_Document.Draft(), "triggers");
    auto rows = Field(document, "triggers").Get_Array();
    const auto matches = [&](const J& row) {
        return String(row, "categoryId") == m_Category &&
            (!collidersOnly || String(Field(row, "event"), "type") == "SPACE_ENTER");
    };
    if (collidersOnly)
    {
        const auto selected = Selected(rows, "triggerId", m_TriggerId);
        if (selected >= rows.size() || !matches(rows[selected]))
        {
            m_TriggerId.clear();
            for (const auto& row : rows) if (matches(row)) { m_TriggerId = String(row, "triggerId"); break; }
        }
        ImGui::TextUnformatted("행을 선택하면 Collider Detail 창에서 위치 / 크기 / 회전을 조절하고 저장할 수 있습니다.");
    }
    if (ImGui::BeginTable("GuideTriggerRows", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 7)))
    {
        ImGui::TableSetupColumn(collidersOnly ? "Collider / Box ID" : "Type / Trigger ID");
        ImGui::TableSetupColumn("Prompt");
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 70.f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        for (auto& row : rows) if (matches(row))
        {
            const auto id = String(row, "triggerId");
            const auto& event = Field(row, "event");
            ImGui::PushID(id.c_str());ImGui::TableNextRow();ImGui::TableNextColumn();
            const auto label = collidersOnly ? String(event, "boxId") : id;
            if (ImGui::Selectable(label.c_str(), m_TriggerId == id))
            {
                m_TriggerId = id;
                m_ColliderOpen = String(event, "type") == "SPACE_ENTER";
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
            if (!collidersOnly) ImGui::TextUnformatted(String(event, "type").c_str());
            ImGui::TableNextColumn();ImGui::TextWrapped("%s", String(row, "promptId").c_str());
            ImGui::TableNextColumn();EditBool("##Enabled", row, "enabled");ImGui::PopID();
        }
        ImGui::EndTable();
    }
    const auto makeBox = [&]() {
        return J::Object({{"type", Str("SPACE_ENTER")}, {"boxId", J::String(New_Id("guide.box"))},
            {"position", Field(Field(m_Document.Draft(), "placement"), "position")},
            {"halfExtents", J::Array({J::Number(3), J::Number(3), J::Number(3)})},
            {"yawDegrees", J::Number(0)}, {"anchorPlacementId", Str("")}});
    };
    if (ImGui::Button(collidersOnly ? "+ Collider" : "+ Trigger"))
    {
        m_TriggerId = New_Id("guide.trigger");
        rows.push_back(J::Object({{"triggerId", J::String(m_TriggerId)}, {"categoryId", J::String(m_Category)},
            {"enabled", J::Boolean(false)}, {"event", collidersOnly ? makeBox() : J::Object({{"type", Str("GUIDE_STARTED")}})},
            {"promptId", Str("")}, {"comboId", Str("")}, {"cooldownMs", J::Number(30000)}, {"priority", J::Number(10)}}));
        if (collidersOnly) m_ColliderOpen = true;
    }
    const auto index = Selected(rows, "triggerId", m_TriggerId);
    if (index < rows.size() && matches(rows[index]))
    {
        auto& row = rows[index];
        auto event = Field(row, "event");
        ImGui::PushID(m_TriggerId.c_str());
        ImGui::TextWrapped("Trigger ID: %s", m_TriggerId.c_str());
        if (!collidersOnly && Choose("Event type", event, "type", {{"GUIDE_STARTED", "Personal guidance starts"},
            {"PARTY_JOINED", "Legacy guidance start"}, {"SPACE_ENTER", "Owner player enters a box"},
            {"RAID_RETURNED", "Returns from a raid to Bern"}, {"WORLD_RETURNED", "Returns from island / Colosseum to Bern"},
            {"BOSS_PATTERN_STARTED", "Boss pattern begins"}, {"HELP_COMMAND", "Help command"}}))
        {
            const auto type = String(event, "type");
            event = J::Object({{"type", J::String(type)}});
            if (type == "SPACE_ENTER") { event = makeBox();m_ColliderOpen = true; }
            else if (type == "BOSS_PATTERN_STARTED") Set(event, "patternId", Str(""));
            else if (type == "HELP_COMMAND") Set(event, "commandId", Str(""));
            else if (type == "RAID_RETURNED") Set(event, "raidWorldId", Str("VALTAN_ARENA"));
            else if (type == "WORLD_RETURNED") Set(event, "sourceWorldId", Str("MAHARAKA"));
        }
        const auto type = String(event, "type");
        if (type == "SPACE_ENTER")
        {
            if (ImGui::Button("Collider Detail")) m_ColliderOpen = true;
            ImGui::TextWrapped("Box: %s", String(event, "boxId").c_str());
        }
        else
        {
            ImGui::TextWrapped("이 이벤트는 콜라이더를 사용하지 않습니다. 공간 배치는 콜라이더 탭에서 편집하세요.");
            if (type == "BOSS_PATTERN_STARTED") Choose("Published pattern", event, "patternId", m_PatternChoices);
            else if (type == "HELP_COMMAND") Choose("Command", event, "commandId", Choices(Field(Field(m_Document.Draft(), "combat"), "commands").Get_Array(), "commandId", "displayName"));
            else if (type == "RAID_RETURNED") Choose("Raid world", event, "raidWorldId", {{"VALTAN_ARENA", "Valtan"}, {"KAKULSAYDON_ARENA", "KoukuSaydon"}});
            else if (type == "WORLD_RETURNED") Choose("Returned world", event, "sourceWorldId", {{"MAHARAKA", "Maharaka"}, {"COLOSSEUM", "Colosseum"}});
        }
        if (!collidersOnly)
        {
            Choose("Prompt", row, "promptId", Choices(Field(Field(m_Document.Draft(), "prompts"), "prompts").Get_Array(), "promptId", "title", true));
            if (type == "HELP_COMMAND") Choose("Combo", row, "comboId", Choices(Field(Field(m_Document.Draft(), "combat"), "combos").Get_Array(), "comboId", "displayName", true));
            else Set(row, "comboId", Str(""));
            EditMilliseconds("Cooldown (ms)", row, "cooldownMs");EditMilliseconds("Priority", row, "priority");
        }
        Set(row, "event", std::move(event));
        if (ImGui::Button("Delete trigger")) { rows.erase(rows.begin() + index);m_TriggerId.clear(); }
        ImGui::PopID();
    }
    else if (collidersOnly) ImGui::TextUnformatted("이 지역에는 공간 트리거가 없습니다. + Collider로 추가하세요.");
    Set(document, "triggers", J::Array(std::move(rows)));Set(m_Document.Draft(), "triggers", std::move(document));
}
void CGuideAITool::Render_ColliderDetail()
{
    if (!m_ColliderOpen || !m_Document.Is_Loaded()) return;
    auto document = Field(m_Document.Draft(), "triggers");
    auto rows = Field(document, "triggers").Get_Array();
    const auto index = Selected(rows, "triggerId", m_TriggerId);
    if (index >= rows.size() || String(rows[index], "categoryId") != m_Category ||
        String(Field(rows[index], "event"), "type") != "SPACE_ENTER")
    { m_ColliderOpen = false;return; }
    ImGui::SetNextWindowSize(ImVec2(680, 620), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Collider Detail##Guide", &m_ColliderOpen))
    {
        m_InteractionRequested |= ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        ImGui::TextWrapped("%s", m_TriggerId.c_str());
        ImGui::Checkbox("Show Debug", &m_ShowBox);ImGui::SameLine();
        ImGui::Checkbox("All colliders in active Area", &m_ShowAllBoxes);
        ImGui::TextWrapped("Draft: yellow = selected, cyan = enabled, gray = disabled.");
        ImGui::Separator();
        ImGui::BeginDisabled(m_Document.Is_Busy() || m_Rewrite);
        ImGui::PushID(m_TriggerId.c_str());
        auto& row = rows[index];
        auto event = Field(row, "event");
        Render_BoxEditor(event);
        Set(row, "event", std::move(event));
        EditBool("Enabled", row, "enabled");
        Choose("Prompt", row, "promptId", Choices(Field(Field(m_Document.Draft(), "prompts"), "prompts").Get_Array(), "promptId", "title", true));
        EditMilliseconds("Cooldown (ms)", row, "cooldownMs");EditMilliseconds("Priority", row, "priority");
        ImGui::PopID();
        // Save must consume this frame's edits, including changes made in the other window.
        Set(document, "triggers", J::Array(std::move(rows)));Set(m_Document.Draft(), "triggers", std::move(document));
        ImGui::Separator();
        if (ImGui::Button("Save")) { Cancel_PlacementPick({});m_Document.Start_Save(); }
        ImGui::SameLine();
        if (ImGui::Button("Publish")) { Cancel_PlacementPick({});m_Document.Start_Publish(); }
        ImGui::SameLine();ImGui::TextUnformatted(m_Document.Is_Dirty() ? "Modified source" : "Saved source");
        ImGui::EndDisabled();
        ImGui::TextWrapped("%s", m_Document.Status().c_str());
        ImGui::TextWrapped("Save writes the Guide source. Publish installs the saved revision; the running Server is separate.");
    }
    ImGui::End();
    if (!m_ColliderOpen) Cancel_PlacementPick({});
}
void CGuideAITool::Render_BoxEditor(J& event)
{
    EditText("Box ID / name", event, "boxId");
    bool moved = EditBoxVector("Position X/Y/Z (m)", event, "position", 1., -100000., 100000.);
    EditBoxVector("Size X/Y/Z (m)", event, "halfExtents", 2., 0.02, 200000.);
    float yaw = static_cast<float>(Number(event, "yawDegrees"));
    if (ImGui::DragFloat("Rotation Y (degrees)", &yaw, 0.5f, -36000.f, 36000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp) && std::isfinite(yaw))
    { Set(event, "yawDegrees", J::Number(yaw, true));moved = true; }
    if (moved) Set(event, "anchorPlacementId", Str(""));
    ImGui::TextUnformatted("Position = box center. Size = full width / height / depth (not half extents).");
    std::string area;
    for (const auto& category : Field(Field(m_Document.Draft(), "catalog"), "categories").Get_Array())
        if (String(category, "categoryId") == m_Category) area = String(category, "areaId");
    ImGui::BeginDisabled(area.empty() || area != m_ActiveArea);
    if (ImGui::Button("Pick box position in world (one click)"))
    {
        m_PickArea = area;m_PickCategory = m_Category;m_PickTriggerId = m_TriggerId;m_PickBoxId = String(event, "boxId");
        m_PickEvent = CGuideAIDocument::Serialize(event);m_PickRequested = true;m_InteractionRequested = true;
        m_ShowBox = true;
        m_Document.Set_Status("Click a visible world surface once to set the box center. Esc / right-click cancels.");
    }
    ImGui::EndDisabled();
    if (!m_PickTriggerId.empty()) { ImGui::SameLine();if (ImGui::Button("Cancel pick")) Cancel_PlacementPick("Pick cancelled; the previous box position was preserved."); }
    if (area != m_ActiveArea) ImGui::TextWrapped("Enter %s to pick or preview this box.", area.c_str());
    if (Choose("Copy NPC position / rotation", event, "anchorPlacementId", m_NpcChoices))
        for (const auto& p : Field(m_NpcWorld, "placements").Get_Array())
            if (String(p, "placementId") == String(event, "anchorPlacementId"))
            { Set(event, "position", Field(p, "position"));Set(event, "yawDegrees", Field(p, "yawDegrees"));break; }
}
void CGuideAITool::Render_Combos()
{
    auto combat=Field(m_Document.Draft(),"combat");auto combos=Field(combat,"combos").Get_Array();
    if(ImGui::BeginCombo("Combo",m_ComboId.c_str())){for(const auto& combo:combos)if(ImGui::Selectable(String(combo,"displayName").c_str(),m_ComboId==String(combo,"comboId")))m_ComboId=String(combo,"comboId");ImGui::EndCombo();}ImGui::SameLine();
    if(ImGui::Button("+ Combo")){m_ComboId=New_Id("combo.dimensionmaster");combos.push_back(J::Object({{"comboId",J::String(m_ComboId)},{"displayName",J::String("콤보 "+std::to_string(combos.size()+1))},{"inputSlots",J::Array({Str("W")})},{"timeoutMs",J::Number(45000)},{"stepWaitMs",J::Number(5000)},{"repeat",J::Boolean(false)}}));}
    const auto index=Selected(combos,"comboId",m_ComboId);if(index<combos.size())
    {
        auto& combo=combos[index];ImGui::TextWrapped("%s",m_ComboId.c_str());EditText("Name",combo,"displayName");EditMilliseconds("Total timeout (ms)",combo,"timeoutMs");EditMilliseconds("Unavailable step wait (ms)",combo,"stepWaitMs");EditBool("Repeat within timeout",combo,"repeat");
        auto steps=Field(combo,"inputSlots").Get_Array();for(size_t i=0;i<steps.size();++i){ImGui::PushID(static_cast<int>(i));ImGui::Text("%zu",i+1);ImGui::SameLine();J wrapper=J::Object({{"slot",steps[i]}});Choose("##Slot",wrapper,"slot",m_SkillChoices);steps[i]=Field(wrapper,"slot");ImGui::SameLine();if(i>0&&ImGui::SmallButton("Up"))std::swap(steps[i],steps[i-1]);ImGui::SameLine();if(i+1<steps.size()&&ImGui::SmallButton("Down"))std::swap(steps[i],steps[i+1]);ImGui::SameLine();if(steps.size()>1&&ImGui::SmallButton("Remove")){steps.erase(steps.begin()+i);ImGui::PopID();break;}ImGui::PopID();}if(steps.size()<32&&ImGui::Button("+ Step"))steps.push_back(Str("W"));Set(combo,"inputSlots",J::Array(std::move(steps)));ImGui::SameLine();if(ImGui::Button("Delete combo")){if(References(m_Document.Draft(),"comboId",m_ComboId))m_Document.Set_Status("Combo is referenced by a command or trigger.");else{combos.erase(combos.begin()+index);m_ComboId.clear();}}
    }
    Set(combat,"combos",J::Array(combos));ImGui::Separator();auto commands=Field(combat,"commands").Get_Array();
    if(ImGui::BeginCombo("Help command",m_CommandId.c_str())){for(const auto& command:commands)if(ImGui::Selectable(String(command,"displayName").c_str(),m_CommandId==String(command,"commandId")))m_CommandId=String(command,"commandId");ImGui::EndCombo();}ImGui::SameLine();if(ImGui::Button("+ Command")){m_CommandId=New_Id("guide.command");commands.push_back(J::Object({{"commandId",J::String(m_CommandId)},{"displayName",Str("새 도움 명령")},{"aliases",J::Array({J::String("도움 "+std::to_string(commands.size()+1))})},{"comboId",combos.empty()?Str(""):Field(combos.front(),"comboId")},{"stop",J::Boolean(combos.empty())},{"enabled",J::Boolean(false)},{"cooldownMs",J::Number(3000)}}));}
    const auto commandIndex=Selected(commands,"commandId",m_CommandId);if(commandIndex<commands.size())
    {
        auto& command=commands[commandIndex];EditText("Command name",command,"displayName");EditBool("Command enabled",command,"enabled");if(EditBool("Stop assistance",command,"stop")&&Boolean(command,"stop"))Set(command,"comboId",Str(""));if(!Boolean(command,"stop"))Choose("Command combo",command,"comboId",Choices(combos,"comboId","displayName"));EditMilliseconds("Command cooldown (ms)",command,"cooldownMs");
        std::string aliases;for(const auto& alias:Field(command,"aliases").Get_Array()){if(!aliases.empty())aliases+='\n';aliases+=alias.Get_String();}J text=J::Object({{"aliases",J::String(aliases)}});if(EditText("Exact phrases (one per line)",text,"aliases",true)){J::ARRAY list;std::istringstream stream(String(text,"aliases"));std::string line;while(std::getline(stream,line))if(!line.empty())list.push_back(J::String(line));Set(command,"aliases",J::Array(std::move(list)));}
        if(ImGui::Button("Delete command")){bool used=false;for(const auto& t:Field(Field(m_Document.Draft(),"triggers"),"triggers").Get_Array())used|=String(Field(t,"event"),"commandId")==m_CommandId;if(used)m_Document.Set_Status("Command is referenced by a trigger.");else{commands.erase(commands.begin()+commandIndex);m_CommandId.clear();}}
    }
    Set(combat,"commands",J::Array(std::move(commands)));Set(m_Document.Draft(),"combat",std::move(combat));
}
void CGuideAITool::Render_DebugBoxes()
{
    if (!m_ShowBox || m_ActiveArea.empty()) return;
    const auto& categories = Field(Field(m_Document.Draft(), "catalog"), "categories").Get_Array();
    for (const auto& row : Field(Field(m_Document.Draft(), "triggers"), "triggers").Get_Array())
    {
        const auto& event = Field(row, "event");
        const bool selected = String(row, "triggerId") == m_TriggerId;
        if (String(event, "type") != "SPACE_ENTER" || (!m_ShowAllBoxes && !selected)) continue;
        for (const auto& category : categories)
            if (String(category, "categoryId") == String(row, "categoryId") && String(category, "areaId") == m_ActiveArea)
            { Render_BoxPreview(event, selected, Boolean(row, "enabled"));break; }
    }
}
void CGuideAITool::Render_BoxPreview(const J& event, bool selected, bool enabled)
{
    const auto& p = Field(event, "position").Get_Array();
    const auto& h = Field(event, "halfExtents").Get_Array();
    if (p.size() != 3 || h.size() != 3 || !std::isfinite(Number(event, "yawDegrees"))) return;
    for (size_t i = 0; i < 3; ++i)
        if (!std::isfinite(p[i].Get_Number()) || !std::isfinite(h[i].Get_Number()) || h[i].Get_Number() <= 0.) return;
    const auto* view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
    const auto* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
    if (!view || !projection) return;
    const auto transform = DirectX::XMMatrixRotationY(static_cast<float>(Number(event, "yawDegrees")) * DirectX::XM_PI / 180.f) *
        DirectX::XMMatrixTranslation(static_cast<float>(p[0].Get_Number()), static_cast<float>(p[1].Get_Number()), static_cast<float>(p[2].Get_Number())) *
        DirectX::XMLoadFloat4x4(view) * DirectX::XMLoadFloat4x4(projection);
    std::array<DirectX::XMFLOAT4, 8> points;
    for (size_t i = 0; i < points.size(); ++i)
        DirectX::XMStoreFloat4(&points[i], DirectX::XMVector4Transform(DirectX::XMVectorSet(
            static_cast<float>(h[0].Get_Number()) * ((i & 1) ? 1.f : -1.f),
            static_cast<float>(h[1].Get_Number()) * ((i & 2) ? 1.f : -1.f),
            static_cast<float>(h[2].Get_Number()) * ((i & 4) ? 1.f : -1.f), 1.f), transform));
    // Clip edges before dividing by W so boxes crossing the camera remain visible.
    const auto planes = [](const DirectX::XMFLOAT4& v) {
        return std::array<float, 7>{v.w - 0.001f, v.z, v.w - v.z, v.x + v.w, v.w - v.x, v.y + v.w, v.w - v.y};
    };
    const auto viewport = ImGui::GetMainViewport();
    const auto project = [&](const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b, float t) {
        const float w = a.w + (b.w - a.w) * t;
        return ImVec2(viewport->Pos.x + ((a.x + (b.x - a.x) * t) / w + 1.f) * .5f * viewport->Size.x,
            viewport->Pos.y + (1.f - (a.y + (b.y - a.y) * t) / w) * .5f * viewport->Size.y);
    };
    const ImU32 color = selected ? IM_COL32(255, 190, 30, 255) :
        enabled ? IM_COL32(50, 220, 255, 220) : IM_COL32(150, 150, 150, 180);
    auto* draw = ImGui::GetBackgroundDrawList();
    ImVec2 labelPosition{};
    bool hasLabel = false;
    draw->PushClipRect(viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y), true);
    for (size_t i = 0; i < points.size(); ++i) for (size_t bit : {size_t(1), size_t(2), size_t(4)}) if (!(i & bit))
    {
        const auto& a = points[i];const auto& b = points[i | bit];
        const auto pa = planes(a);const auto pb = planes(b);
        float start = 0.f, end = 1.f;
        bool visible = true;
        for (size_t plane = 0; plane < pa.size(); ++plane)
        {
            if (!std::isfinite(pa[plane]) || !std::isfinite(pb[plane]) || (pa[plane] < 0.f && pb[plane] < 0.f)) { visible = false;break; }
            if (pa[plane] < 0.f) start = (std::max)(start, pa[plane] / (pa[plane] - pb[plane]));
            else if (pb[plane] < 0.f) end = (std::min)(end, pa[plane] / (pa[plane] - pb[plane]));
        }
        if (!visible || start > end) continue;
        const auto first = project(a, b, start), last = project(a, b, end);
        draw->AddLine(first, last, color, selected ? 3.f : 1.5f);
        if (!hasLabel || first.y < labelPosition.y) { labelPosition = first;hasLabel = true; }
    }
    if (hasLabel)
    {
        const auto label = String(event, "boxId") + (enabled ? "" : " [disabled]");
        const auto size = ImGui::CalcTextSize(label.c_str());
        labelPosition.x = (std::max)(viewport->Pos.x, (std::min)(labelPosition.x, viewport->Pos.x + viewport->Size.x - size.x - 8.f));
        labelPosition.y = (std::max)(viewport->Pos.y, labelPosition.y - size.y - 6.f);
        draw->AddRectFilled(labelPosition, ImVec2(labelPosition.x + size.x + 8.f, labelPosition.y + size.y + 4.f), IM_COL32(0, 0, 0, 190));
        draw->AddText(ImVec2(labelPosition.x + 4.f, labelPosition.y + 2.f), color, label.c_str());
    }
    draw->PopClipRect();
}
}
#endif
