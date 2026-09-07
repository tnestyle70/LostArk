#include "imgui.h"

#include "EffectAuthoringResourceTree.h"
#include "DataJson.h"
#include "EffectV2_Catalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <fstream>
#include <iterator>
#include <set>

namespace
{
    using namespace Client;
    using OWNER = EFFECT_RESOURCE_OWNER_KIND;
    constexpr size_t MAX_NODES = 4096u;
    constexpr size_t MAX_REFERENCES = 16384u;

    bool Fail(std::string& Error, std::string Message)
    {
        Error = std::move(Message);
        return false;
    }

    bool Stable_Id(const std::string& Id, size_t Maximum = 160u)
    {
        return !Id.empty() && Id.size() <= Maximum &&
            std::all_of(Id.begin(), Id.end(), [](unsigned char C)
            { return (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
                (C >= '0' && C <= '9') || C == '.' || C == '_' || C == '-'; });
    }

    bool Name(const std::string& Value)
    {
        return !Value.empty() && CEffectV2Document::Is_ValidDisplayName(Value) &&
            Value.find_first_not_of(" \t\r\n") != std::string::npos;
    }

    const char* Kind_Key(OWNER Kind)
    {
        switch (Kind)
        {
        case OWNER::V1_DOCUMENT: return "V1";
        case OWNER::V2_LEAF: return "V2_LEAF";
        case OWNER::V2_GROUP: return "V2_GROUP";
        default: return "";
        }
    }

    OWNER Parse_Kind(const std::string& Key)
    {
        if (Key == "V1") return OWNER::V1_DOCUMENT;
        if (Key == "V2_LEAF") return OWNER::V2_LEAF;
        if (Key == "V2_GROUP") return OWNER::V2_GROUP;
        return OWNER::END;
    }

    std::string Root_ForKind(OWNER Kind)
    {
        if (Kind == OWNER::V1_DOCUMENT) return "root.v1";
        if (Kind == OWNER::V2_LEAF || Kind == OWNER::V2_GROUP) return "root.v2";
        return {};
    }

    std::string New_Id(const char* Prefix)
    {
        static std::atomic<uint64_t> Serial{ 0u };
        return std::string(Prefix) + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()) + "." +
            std::to_string(GetCurrentProcessId()) + "." + std::to_string(++Serial);
    }

    std::string Quoted(const std::string& Value)
    {
        return "\"" + CDataJson::Escape(Value) + "\"";
    }

    bool Fields(const DATA_JSON_VALUE& Value, std::initializer_list<const char*> Keys)
    {
        if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size()) return false;
        for (const char* Key : Keys) if (!Value.Find(Key)) return false;
        return true;
    }

    bool String_Field(const DATA_JSON_VALUE& Value, const char* Key, std::string& Out)
    {
        const auto* Field = Value.Find(Key);
        if (!Field || !Field->Is_String()) return false;
        Out = Field->Get_String();
        return true;
    }

    bool Contains(const std::string& Text, const std::string& Search)
    {
        return std::search(Text.begin(), Text.end(), Search.begin(), Search.end(),
            [](unsigned char A, unsigned char B) { return std::tolower(A) == std::tolower(B); }) != Text.end();
    }
}

bool Client::CEffectAuthoringResourceTree::Read_Source(const std::filesystem::path& Path,
    std::string& Bytes, bool& Exists, std::string& Error)
{
    std::error_code Code;
    if (Path.empty()) return Fail(Error, "Effect source path is unavailable.");
    Exists = std::filesystem::exists(Path, Code);
    if (Code) return Fail(Error, "Cannot inspect effect source: " + Code.message());
    Bytes.clear();
    if (!Exists) return true;
    const auto Size = std::filesystem::file_size(Path, Code);
    if (Code || Size > 16u * 1024u * 1024u)
        return Fail(Error, "Effect source is unreadable or exceeds the 16 MiB JSON limit.");
    std::ifstream Input(Path, std::ios::binary);
    if (!Input.is_open()) return Fail(Error, "Cannot open effect source: " + Path.string());
    Bytes.assign(std::istreambuf_iterator<char>(Input), std::istreambuf_iterator<char>());
    if (Input.bad()) return Fail(Error, "Cannot read effect source: " + Path.string());
    return true;
}

std::string Client::CEffectAuthoringResourceTree::Root_For(const DOCUMENT& Document,
    const std::string& NodeId)
{
    std::string Current = NodeId;
    std::set<std::string> Visited;
    for (size_t Depth = 0u; Depth <= 32u; ++Depth)
    {
        if (Current == "root.v1" || Current == "root.v2") return Current;
        if (!Visited.insert(Current).second) return {};
        const auto Found = std::find_if(Document.Nodes.begin(), Document.Nodes.end(),
            [&](const NODE& Node) { return Node.strId == Current; });
        if (Found == Document.Nodes.end()) return {};
        Current = Found->strParentId;
    }
    return {};
}

bool Client::CEffectAuthoringResourceTree::Validate(const DOCUMENT& Document, std::string& Error)
{
    if (Document.Nodes.size() > MAX_NODES || Document.References.size() > MAX_REFERENCES)
        return Fail(Error, "Saved Effects tree exceeds the supported item count.");
    std::set<std::string> Ids;
    for (const NODE& Node : Document.Nodes)
    {
        if (!Stable_Id(Node.strId) || Node.strId.starts_with("root.") ||
            !Stable_Id(Node.strParentId) || !Name(Node.strDisplayName) || !Ids.insert(Node.strId).second)
            return Fail(Error, "Tree parent has an invalid or duplicate stable ID or name.");
        if (Root_For(Document, Node.strId).empty())
            return Fail(Error, "Tree parent is missing, cyclic, or deeper than 32 levels.");
    }
    std::set<std::pair<OWNER, std::string>> Resources;
    for (const REFERENCE& Ref : Document.References)
    {
        const auto Root = Root_ForKind(Ref.eKind);
        if (Root.empty() || !Stable_Id(Ref.strAssetId, Ref.eKind == OWNER::V2_LEAF ? 80u : 160u) ||
            (!Ref.strDisplayName.empty() && !Name(Ref.strDisplayName)) ||
            Root_For(Document, Ref.strParentId) != Root ||
            !Resources.emplace(Ref.eKind, Ref.strAssetId).second)
            return Fail(Error, "Tree reference has an invalid identity, duplicate resource, or wrong V1/V2 parent.");
    }
    return true;
}

bool Client::CEffectAuthoringResourceTree::Parse(const std::string& Bytes,
    DOCUMENT& OutDocument, std::string& Error)
{
    DATA_JSON_VALUE Root;
    if (!CDataJson::Parse(Bytes, Root, Error)) return false;
    if (!Fields(Root, { "schema", "formatVersion", "nodes", "references" }))
        return Fail(Error, "Saved Effects tree has missing or unknown fields.");
    const auto* Schema = Root.Find("schema");
    const auto* Version = Root.Find("formatVersion");
    const auto* Nodes = Root.Find("nodes");
    const auto* References = Root.Find("references");
    if (!Schema->Is_String() || Schema->Get_String() != "lostark.effect-resource-tree" ||
        !Version->Is_Number() || Version->Get_Number() != 1.0 ||
        !Nodes->Is_Array() || !References->Is_Array())
        return Fail(Error, "Saved Effects tree schema/version or arrays are invalid.");
    DOCUMENT Staged;
    for (const auto& Value : Nodes->Get_Array())
    {
        NODE Node;
        std::string Kind;
        if (!Fields(Value, { "id", "parentId", "kind", "displayName" }) ||
            !String_Field(Value, "id", Node.strId) || !String_Field(Value, "parentId", Node.strParentId) ||
            !String_Field(Value, "displayName", Node.strDisplayName) || !String_Field(Value, "kind", Kind) ||
            (Kind != "CATEGORY" && Kind != "PARENT"))
            return Fail(Error, "Saved Effects parent metadata is invalid.");
        Node.bCategory = Kind == "CATEGORY";
        Staged.Nodes.push_back(std::move(Node));
    }
    for (const auto& Value : References->Get_Array())
    {
        REFERENCE Ref;
        std::string Kind;
        if (!Fields(Value, { "kind", "assetId", "displayName", "parentId" }) ||
            !String_Field(Value, "kind", Kind) || !String_Field(Value, "assetId", Ref.strAssetId) ||
            !String_Field(Value, "displayName", Ref.strDisplayName) || !String_Field(Value, "parentId", Ref.strParentId))
            return Fail(Error, "Saved Effects resource reference is invalid.");
        Ref.eKind = Parse_Kind(Kind);
        Staged.References.push_back(std::move(Ref));
    }
    if (!Validate(Staged, Error)) return false;
    OutDocument = std::move(Staged);
    return true;
}

std::string Client::CEffectAuthoringResourceTree::Serialize(const DOCUMENT& Document)
{
    std::string Bytes = "{\n  \"schema\": \"lostark.effect-resource-tree\",\n  \"formatVersion\": 1,\n  \"nodes\": [";
    for (size_t Index = 0u; Index < Document.Nodes.size(); ++Index)
    {
        const NODE& Node = Document.Nodes[Index];
        Bytes += (Index ? ",\n" : "\n");
        Bytes += "    {\"id\": " + Quoted(Node.strId) + ", \"parentId\": " + Quoted(Node.strParentId) +
            ", \"kind\": " + Quoted(Node.bCategory ? "CATEGORY" : "PARENT") +
            ", \"displayName\": " + Quoted(Node.strDisplayName) + "}";
    }
    Bytes += "\n  ],\n  \"references\": [";
    for (size_t Index = 0u; Index < Document.References.size(); ++Index)
    {
        const REFERENCE& Ref = Document.References[Index];
        Bytes += (Index ? ",\n" : "\n");
        Bytes += "    {\"kind\": " + Quoted(Kind_Key(Ref.eKind)) + ", \"assetId\": " + Quoted(Ref.strAssetId) +
            ", \"displayName\": " + Quoted(Ref.strDisplayName) + ", \"parentId\": " + Quoted(Ref.strParentId) + "}";
    }
    return Bytes + "\n  ]\n}\n";
}

bool Client::CEffectAuthoringResourceTree::Read_V1Inventory(std::vector<RESOURCE>& OutRows, std::string& Error)
{
    const auto Directory = CProjectDataRoot::Resolve("Effects/Authored");
    std::error_code Code;
    const bool Exists = std::filesystem::exists(Directory, Code);
    if (Code) return Fail(Error, "Cannot inspect V1 saved effects: " + Code.message());
    std::vector<RESOURCE> Staged;
    if (Exists)
    {
        std::filesystem::directory_iterator It(Directory, Code), End;
        if (Code) return Fail(Error, "Cannot list V1 saved effects: " + Code.message());
        for (; It != End; It.increment(Code))
        {
            if (Code) return Fail(Error, "V1 saved effect scan failed: " + Code.message());
            if (!It->is_regular_file(Code))
            {
                if (Code) return Fail(Error, "Cannot inspect V1 saved effect: " + Code.message());
                continue;
            }
            const std::string File = It->path().filename().string();
            constexpr std::string_view Suffix = ".effect.json";
            if (!File.ends_with(Suffix)) continue;
            RESOURCE Row;
            Row.eKind = OWNER::V1_DOCUMENT;
            Row.strAssetId = File.substr(0u, File.size() - Suffix.size());
            std::string Bytes;
            bool Present = false;
            DATA_JSON_VALUE Root;
            if (!Stable_Id(Row.strAssetId)) Row.strStatus = "Source filename has an invalid stable ID.";
            else if (Read_Source(It->path(), Bytes, Present, Row.strStatus) && Present &&
                CDataJson::Parse(Bytes, Root, Row.strStatus))
            {
                const auto* Id = Root.Find("effectAssetId");
                const auto* Schema = Root.Find("schema");
                if (!Root.Is_Object() || !Id || !Id->Is_String() || Id->Get_String() != Row.strAssetId ||
                    !Schema || !Schema->Is_String() || Schema->Get_String() != "lostark.effect-authoring")
                    Row.strStatus = "V1 schema or filename identity mismatch.";
                if (const auto* Label = Root.Find("displayName"))
                {
                    if (Label->Is_String() && Name(Label->Get_String())) Row.strDisplayName = Label->Get_String();
                    else Row.strStatus = "V1 displayName is invalid.";
                }
            }
            else if (!Present && Row.strStatus.empty()) Row.strStatus = "Source disappeared during discovery.";
            Staged.push_back(std::move(Row));
        }
        if (Code) return Fail(Error, "V1 saved effect scan failed: " + Code.message());
    }
    std::sort(Staged.begin(), Staged.end(), [](const auto& A, const auto& B) { return A.strAssetId < B.strAssetId; });
    OutRows = std::move(Staged);
    return true;
}

bool Client::CEffectAuthoringResourceTree::Reload(std::string& Status)
{
    m_bReloadAttempted = true;
    std::string Bytes, Error;
    bool Exists = false;
    DOCUMENT Staged;
    const bool MetadataReady = Read_Source(CProjectDataRoot::Resolve("Effects/EffectResourceTree.json"),
        Bytes, Exists, Error) && (!Exists || Parse(Bytes, Staged, Error));
    if (MetadataReady)
    {
        m_Document = std::move(Staged);
        m_strBaselineBytes = std::move(Bytes);
        m_bBaselineExists = Exists;
        m_bMetadataReady = true;
        if (Root_For(m_Document, m_strSelectedParentId).empty()) m_strSelectedParentId = "root.v1";
    }
    Status = MetadataReady ? "Saved Effects metadata ready." : "Previous tree preserved: " + Error;
    std::string InventoryError;
    const bool V1Ready = Read_V1Inventory(m_V1Resources, InventoryError);
    if (!V1Ready) Status += " V1 previous list preserved: " + InventoryError;
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> V2;
    const bool V2Ready = CEffectV2Catalog::Get().Read_Inventory(V2, InventoryError);
    if (V2Ready)
    {
        std::vector<RESOURCE> Rows;
        for (const auto& Row : V2)
            Rows.push_back({ Row.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? OWNER::V2_GROUP : OWNER::V2_LEAF,
                Row.strResourceId, Row.strDisplayName, Row.strStatus });
        m_V2Resources = std::move(Rows);
    }
    else Status += " V2 previous list preserved: " + InventoryError;
    m_strStatus = Status;
    return MetadataReady && V1Ready && V2Ready;
}

bool Client::CEffectAuthoringResourceTree::Save_Staged(DOCUMENT Candidate, std::string& Status)
{
    if (!m_bMetadataReady) return Fail(Status, "Load valid tree metadata before saving; existing source is preserved.");
    if (!Validate(Candidate, Status)) return false;
    const std::string Bytes = Serialize(Candidate);
    DOCUMENT Parsed;
    if (!Parse(Bytes, Parsed, Status)) return false;
    const auto Path = CProjectDataRoot::Resolve("Effects/EffectResourceTree.json");
    std::string Current;
    bool Exists = false;
    if (!Read_Source(Path, Current, Exists, Status)) return false;
    if (Exists != m_bBaselineExists || Current != m_strBaselineBytes)
        return Fail(Status, "Tree metadata changed or was deleted on disk. Reload before saving; previous tree is preserved.");
    if (!CEffectV2Document::Write_AtomicFile(Path, Bytes, Status)) return false;
    m_Document = std::move(Parsed);
    m_strBaselineBytes = Bytes;
    m_bBaselineExists = true;
    Status = "Saved Effects tree saved.";
    return true;
}

bool Client::CEffectAuthoringResourceTree::Attach_Saved(OWNER Kind, const std::string& AssetId,
    const std::string& DisplayName, const std::string& ParentId, std::string& Status)
{
    if (!m_bReloadAttempted) Reload(Status);
    DOCUMENT Candidate = m_Document;
    const auto Found = std::find_if(Candidate.References.begin(), Candidate.References.end(),
        [&](const REFERENCE& Ref) { return Ref.eKind == Kind && Ref.strAssetId == AssetId; });
    const std::string Parent = !ParentId.empty() ? ParentId :
        Found != Candidate.References.end() ? Found->strParentId : Root_ForKind(Kind);
    REFERENCE Ref{ Kind, AssetId, DisplayName, Parent };
    if (Found == Candidate.References.end()) Candidate.References.push_back(std::move(Ref));
    else *Found = std::move(Ref);
    const bool Saved = Save_Staged(std::move(Candidate), Status);
    m_strStatus = Status;
    if (Saved)
    {
        m_strSelectedParentId = Parent;
        m_SelectedResource = { Kind, AssetId };
        // Refresh discovery only at this explicit successful-save edge.
        std::string RefreshStatus;
        Reload(RefreshStatus);
        m_strStatus = Status;
    }
    return Saved;
}

bool Client::CEffectAuthoringResourceTree::Create_Parent(bool Category)
{
    if (!Name(m_szName)) return Fail(m_strStatus, "Enter a nonempty UTF-8 name of at most 256 bytes.");
    DOCUMENT Candidate = m_Document;
    const std::string Id = New_Id("tree.");
    Candidate.Nodes.push_back({ Id, m_strSelectedParentId, m_szName, Category });
    if (!Save_Staged(std::move(Candidate), m_strStatus)) return false;
    m_strSelectedParentId = Id;
    m_SelectedResource = {};
    m_szName[0] = '\0';
    return true;
}

std::string Client::CEffectAuthoringResourceTree::Selected_ParentName() const
{
    if (m_strSelectedParentId == "root.v1") return "V1";
    if (m_strSelectedParentId == "root.v2") return "V2";
    for (const auto& Node : m_Document.Nodes)
        if (Node.strId == m_strSelectedParentId) return Node.strDisplayName;
    return {};
}

Client::EFFECT_RESOURCE_OWNER_KIND Client::CEffectAuthoringResourceTree::Selected_Kind() const
{
    const auto Root = Root_For(m_Document, m_strSelectedParentId);
    if (Root == "root.v1") return OWNER::V1_DOCUMENT;
    if (Root == "root.v2") return m_eCreateKind;
    return OWNER::END;
}

const Client::CEffectAuthoringResourceTree::REFERENCE* Client::CEffectAuthoringResourceTree::Find_Reference(
    OWNER Kind, const std::string& Id) const
{
    for (const REFERENCE& Ref : m_Document.References)
        if (Ref.eKind == Kind && Ref.strAssetId == Id) return &Ref;
    return nullptr;
}

void Client::CEffectAuthoringResourceTree::Queue_Selected(COMMAND_KIND Kind)
{
    if (m_Commands.size() >= 32u) { m_strStatus = "Process the pending resource command first."; return; }
    for (const auto* Rows : { &m_V1Resources, &m_V2Resources })
        for (const RESOURCE& Row : *Rows)
        {
            if (Row.eKind != m_SelectedResource.eOwnerKind || Row.strAssetId != m_SelectedResource.strStableId) continue;
            if (!Row.strStatus.empty()) { m_strStatus = Row.strStatus; return; }
            const auto* Ref = Find_Reference(Row.eKind, Row.strAssetId);
            const std::string Label = Ref && !Ref->strDisplayName.empty() ? Ref->strDisplayName : Row.strDisplayName;
            m_Commands.push_back({ Kind, Row.eKind, Row.strAssetId, Label.empty() ? Row.strAssetId : Label,
                Ref ? Ref->strParentId : Root_ForKind(Row.eKind) });
            return;
        }
    m_strStatus = "The selected saved source is unavailable. Reload after restoring it.";
}

bool Client::CEffectAuthoringResourceTree::Take_Command(COMMAND& Out)
{
    if (m_Commands.empty()) return false;
    Out = std::move(m_Commands.front());
    m_Commands.pop_front();
    return true;
}

void Client::CEffectAuthoringResourceTree::Render_Resources(const std::string& ParentId)
{
    std::set<std::pair<OWNER, std::string>> Seen;
    const auto Draw = [&](OWNER Kind, const std::string& Id, const std::string& NameValue,
        const std::string& Error)
    {
        const std::string Label = NameValue.empty() ? Id : NameValue;
        if (m_szSearch[0] && !Contains(Label, m_szSearch) && !Contains(Id, m_szSearch)) return;
        ImGui::PushID(Kind_Key(Kind));
        ImGui::PushID(Id.c_str());
        const bool Selected = m_SelectedResource.eOwnerKind == Kind && m_SelectedResource.strStableId == Id;
        std::string Visible = Label + (Kind == OWNER::V2_GROUP ? " (Group)" : "") + (!Error.empty() ? " [!]" : "");
        if (ImGui::Selectable(Visible.c_str(), Selected))
        {
            m_SelectedResource = { Kind, Id };
            m_strSelectedParentId = ParentId;
            if (Error.empty()) Queue_Selected(COMMAND_KIND::OPEN);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(Id.c_str());
            if (!Error.empty()) ImGui::TextWrapped("%s", Error.c_str());
            ImGui::EndTooltip();
        }
        ImGui::PopID();
        ImGui::PopID();
    };
    for (const auto* Rows : { &m_V1Resources, &m_V2Resources })
        for (const RESOURCE& Row : *Rows)
        {
            const auto* Ref = Find_Reference(Row.eKind, Row.strAssetId);
            if ((Ref ? Ref->strParentId : Root_ForKind(Row.eKind)) != ParentId) continue;
            Seen.emplace(Row.eKind, Row.strAssetId);
            Draw(Row.eKind, Row.strAssetId, Ref && !Ref->strDisplayName.empty() ? Ref->strDisplayName : Row.strDisplayName, Row.strStatus);
        }
    for (const REFERENCE& Ref : m_Document.References)
        if (Ref.strParentId == ParentId && !Seen.contains({ Ref.eKind, Ref.strAssetId }))
            Draw(Ref.eKind, Ref.strAssetId, Ref.strDisplayName, "Saved source is missing; its tree reference was preserved.");
}

void Client::CEffectAuthoringResourceTree::Render_Branch(const std::string& Id,
    const std::string& Label, bool Root)
{
    ImGui::PushID(Id.c_str());
    ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (Root) Flags |= ImGuiTreeNodeFlags_DefaultOpen;
    if (m_SelectedResource.eOwnerKind == OWNER::END && m_strSelectedParentId == Id)
        Flags |= ImGuiTreeNodeFlags_Selected;
    const bool Open = ImGui::TreeNodeEx("##Parent", Flags, "%s", Label.c_str());
    if (ImGui::IsItemClicked()) { m_strSelectedParentId = Id; m_SelectedResource = {}; }
    if (Open)
    {
        for (const NODE& Node : m_Document.Nodes)
            if (Node.strParentId == Id) Render_Branch(Node.strId, Node.strDisplayName, false);
        if (Root)
        {
            if (ImGui::TreeNodeEx("Uncategorized", ImGuiTreeNodeFlags_DefaultOpen))
            { Render_Resources(Id); ImGui::TreePop(); }
        }
        else Render_Resources(Id);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Client::CEffectAuthoringResourceTree::Render()
{
    if (!m_bReloadAttempted) Reload(m_strStatus);
    ImGui::SeparatorText("Saved Effects");
    if (ImGui::Button("Reload##SavedEffectTree")) Reload(m_strStatus);
    ImGui::SameLine();
    ImGui::TextDisabled("V1 %zu | V2 %zu", m_V1Resources.size(), m_V2Resources.size());
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##SavedEffectSearch", "Search saved name or ID", m_szSearch, sizeof(m_szSearch));
    ImGui::BeginChild("##SavedEffectTree", ImVec2(0.f, 280.f), ImGuiChildFlags_Borders);
    Render_Branch("root.v1", "V1", true);
    Render_Branch("root.v2", "V2", true);
    ImGui::EndChild();
    ImGui::TextWrapped("Parent: %s", Selected_ParentName().c_str());
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##SavedEffectName", "Name", m_szName, sizeof(m_szName));
    ImGui::BeginDisabled(!m_bMetadataReady || !Name(m_szName));
    if (ImGui::Button("Create Parent")) Create_Parent(false);
    ImGui::EndDisabled();
    if (ImGui::CollapsingHeader("Advanced##SavedEffectTree"))
    {
        ImGui::BeginDisabled(!m_bMetadataReady || !Name(m_szName));
        if (ImGui::Button("Create Category")) Create_Parent(true);
        ImGui::EndDisabled();
    }
    if (Root_For(m_Document, m_strSelectedParentId) == "root.v2")
    {
        int Kind = m_eCreateKind == OWNER::V2_GROUP ? 1 : 0;
        if (ImGui::Combo("Create kind", &Kind, "Effect\0Group\0"))
            m_eCreateKind = Kind == 1 ? OWNER::V2_GROUP : OWNER::V2_LEAF;
    }
    ImGui::BeginDisabled(!Name(m_szName) || Selected_Kind() == OWNER::END || m_Commands.size() >= 32u);
    if (ImGui::Button("Create Effect"))
        m_Commands.push_back({ COMMAND_KIND::CREATE_EFFECT, Selected_Kind(), New_Id("effect.user."),
            m_szName, m_strSelectedParentId });
    ImGui::EndDisabled();
    if (!m_strStatus.empty()) ImGui::TextWrapped("%s", m_strStatus.c_str());
}
