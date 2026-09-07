#include "EffectEditingSession.h"
#include "EffectV2_Runtime.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <set>

namespace Client
{
namespace
{
bool Group_Id(const std::string& id)
{
    return !id.empty() && id.size() <= 160u && std::all_of(id.begin(), id.end(), [](unsigned char c)
    { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-'; });
}
bool Read(const std::filesystem::path& path, std::string& text, bool& exists, std::string& error)
{
    std::error_code ec;
    exists = std::filesystem::exists(path, ec);
    if (ec) { error = ec.message(); return false; }
    text.clear();
    if (!exists) return true;
    std::ifstream stream(path, std::ios::binary);
    if (!stream) { error = "Cannot read " + path.generic_string(); return false; }
    text.assign(std::istreambuf_iterator<char>(stream), {});
    if (stream.bad()) { error = "Read failed: " + path.generic_string(); return false; }
    return true;
}
}

std::string CEffectEditingSession::New_Id(const char* prefix)
{
    static unsigned ordinal = 0;
    const auto ticks = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return std::string(prefix) + std::to_string(ticks) + "." + std::to_string(++ordinal);
}

void CEffectEditingSession::Reset()
{
    m_Group = {}; m_Leaves.clear(); m_SelectedChild.clear(); m_GroupBaseline.clear();
    m_GroupExisted = m_Dirty = m_NativeLeaf = false; m_NativeLeafId.clear();
}

bool CEffectEditingSession::Revert(std::string& status)
{
    const auto leaf = m_Leaves.find(m_NativeLeafId);
    const bool existed = m_NativeLeaf ? leaf != m_Leaves.end() && leaf->second.existed : m_GroupExisted;
    if (!existed) { Reset(); status = "Discarded unsaved Effect draft."; return true; }
    CEffectEditingSession staged;
    if (!staged.Load(Resource_Kind(), Resource_Id(), status))
    {
        status = "Current draft preserved; saved Effect could not be reloaded: " + status;
        return false;
    }
    *this = std::move(staged);
    status = "Reloaded saved Effect.";
    return true;
}

void CEffectEditingSession::Create(const std::string& name, const EFFECT_V2_TYPE type,
    const EFFECT_V2_RESOURCE_KIND kind)
{
    Reset();
    m_Group.strGroupId = New_Id("effect.composition.");
    m_Group.strDisplayName = name;
    m_Group.iDurationMs = 5000;
    Add_Element(type, name);
    if (kind == EFFECT_V2_RESOURCE_KIND::LEAF && !m_Group.Children.empty())
    {
        m_NativeLeaf = true;
        m_NativeLeafId = m_Group.Children.front().strEffectId;
    }
}

bool CEffectEditingSession::Promote_ToGroup(std::string& status)
{
    if (Empty() || !m_NativeLeaf) { status = "Select a native Effect before creating its group."; return false; }
    m_Group.strGroupId = New_Id("effect.composition.");
    if (const auto* document = Selected_Document()) m_Group.strDisplayName = document->strDisplayName;
    m_GroupBaseline.clear(); m_GroupExisted = false;
    m_NativeLeaf = false; m_NativeLeafId.clear(); Touch();
    status = "Created an unsaved group referencing this Effect. Its original native ID and file are retained.";
    return true;
}

void CEffectEditingSession::Add_Element(const EFFECT_V2_TYPE type, const std::string& name)
{
    if (Empty() || m_NativeLeaf || type == EFFECT_V2_TYPE::END) return;
    LEAF leaf;
    leaf.document.strEffectId = New_Id("effect.element.");
    leaf.document.strDisplayName = name;
    leaf.document.eType = type;
    leaf.document.Desc.eShape = CEffectV2Document::Shape_ForType(type);
    leaf.document.Desc.bParamsAuthored = true;
    leaf.document.Desc.Params.fLifetime = 3.f;
    leaf.document.Desc.Params.bLoop = false;
    leaf.document.Desc.Params.Particle.bLocalSpace = false;
    EFFECT_V2_GROUP_CHILD child;
    child.strChildId = New_Id("element.");
    child.strEffectId = child.strResourceId = leaf.document.strEffectId;
    child.iDurationMs = 3000;
    m_SelectedChild = child.strChildId;
    m_Leaves.emplace(leaf.document.strEffectId, std::move(leaf));
    m_Group.Children.push_back(std::move(child));
    Touch();
}

bool CEffectEditingSession::Load(const EFFECT_V2_RESOURCE_KIND kind, const std::string& id, std::string& status)
{
    if (Dirty()) { status = "Save or Revert the current Effect before opening another resource."; return false; }
    CEffectEditingSession staged;
    if (kind == EFFECT_V2_RESOURCE_KIND::GROUP)
    {
        if (!Group_Id(id)) { status = "Invalid Effect group ID."; return false; }
        if (!Read(CEffectV2Document::Group_Path(id), staged.m_GroupBaseline, staged.m_GroupExisted, status) ||
            !staged.m_GroupExisted || !CEffectV2Document::Parse_Group(staged.m_GroupBaseline, staged.m_Group, status)) return false;
        if (staged.m_Group.strGroupId != id) { status = "Group file identity does not match its resource ID."; return false; }
        for (const auto& child : staged.m_Group.Children)
        {
            if (child.eResourceKind != EFFECT_V2_RESOURCE_KIND::LEAF)
            { status = "This group requires the original nested-resource editor."; return false; }
            LEAF leaf;
            const auto& leafId = child.strEffectId;
            if (!Read(CEffectV2Document::Document_Path(leafId), leaf.baseline, leaf.existed, status) ||
                !leaf.existed || !CEffectV2Document::Parse_Document(leaf.baseline, leaf.document, status)) return false;
            if (leaf.document.strEffectId != leafId) { status = "Element file identity mismatch: " + leafId; return false; }
            staged.m_Leaves.emplace(leafId, std::move(leaf));
        }
    }
    else if (kind == EFFECT_V2_RESOURCE_KIND::LEAF)
    {
        // The temporary child supplies selection to shared tuning controls.
        // Native leaf Save never writes this in-memory group.
        LEAF leaf;
        if (!CEffectV2Document::Is_ValidEffectId(id)) { status = "Invalid native Effect ID."; return false; }
        if (!Read(CEffectV2Document::Document_Path(id), leaf.baseline, leaf.existed, status)) return false;
        if (!leaf.existed) { status = "Native Effect source is missing: " + id; return false; }
        if (!CEffectV2Document::Parse_Document(leaf.baseline, leaf.document, status)) return false;
        if (leaf.document.strEffectId != id) { status = "Element file identity mismatch."; return false; }
        staged.m_Group.strGroupId = New_Id("effect.composition.");
        staged.m_Group.iDurationMs = 0;
        staged.m_Group.strDisplayName = leaf.document.strDisplayName;
        EFFECT_V2_GROUP_CHILD child;
        child.strChildId = New_Id("element.");
        child.strEffectId = child.strResourceId = id;
        staged.m_SelectedChild = child.strChildId;
        staged.m_Group.Children.push_back(std::move(child));
        staged.m_Leaves.emplace(id, std::move(leaf));
        staged.m_NativeLeaf = true;
        staged.m_NativeLeafId = id;
    }
    else { status = "Unsupported Effect resource kind."; return false; }
    if (staged.m_SelectedChild.empty() && !staged.m_Group.Children.empty())
        staged.m_SelectedChild = staged.m_Group.Children.front().strChildId;
    *this = std::move(staged);
    status = "Loaded document draft; playback validates only this Effect's resources.";
    return true;
}

bool CEffectEditingSession::Append(const std::string& id, std::string& status)
{
    if (Empty()) { status = "Create or open an Effect first."; return false; }
    if (m_NativeLeaf) { status = "Create a group explicitly before appending another Effect."; return false; }
    if (!CEffectV2Document::Is_ValidEffectId(id)) { status = "Invalid native Effect ID."; return false; }
    const bool newLeaf = !m_Leaves.contains(id);
    if (newLeaf)
    {
        LEAF leaf;
        if (!Read(CEffectV2Document::Document_Path(id), leaf.baseline, leaf.existed, status) ||
            !leaf.existed || !CEffectV2Document::Parse_Document(leaf.baseline, leaf.document, status)) return false;
        if (leaf.document.strEffectId != id) { status = "Element file identity mismatch."; return false; }
        m_Leaves.emplace(id, std::move(leaf));
    }
    EFFECT_V2_GROUP_CHILD child;
    child.strChildId = New_Id("element.");
    child.strEffectId = child.strResourceId = id;
    const auto& document = m_Leaves.at(id).document;
    const auto& params = document.Desc.Params;
    const float rate = (std::max)(.001f, params.fPlayRate);
    const double duration = params.fLifetime > 0.f ?
        (std::max)(1.0, std::ceil(double(params.fLifetime) * 1000.0 / rate)) : 3000.0;
    double tail = 0.0;
    if (document.eType == EFFECT_V2_TYPE::PARTICLE)
    { child.eStop = EFFECT_V2_CHILD_STOP::DEACTIVATE; tail = double(params.Particle.vLifetime.y) / rate; }
    else if (document.eType == EFFECT_V2_TYPE::TRAIL)
    { child.eStop = EFFECT_V2_CHILD_STOP::DEACTIVATE; tail = double(params.Trail.fPointLifetime) / rate; }
    const double endMs = duration + std::ceil(tail * 1000.0);
    if (!std::isfinite(endMs) || endMs > 600000.0)
    {
        if (newLeaf) m_Leaves.erase(id);
        status = "Element emission and remaining lifetime exceed the ten-minute timeline."; return false;
    }
    child.iDurationMs = static_cast<uint32_t>(duration);
    m_Group.iDurationMs = (std::max)(m_Group.iDurationMs, static_cast<uint32_t>(endMs));
    m_SelectedChild = child.strChildId;
    m_Group.Children.push_back(std::move(child));
    Touch();
    return true;
}

EFFECT_V2_GROUP_CHILD* CEffectEditingSession::Selected_Child()
{
    const auto found = std::find_if(m_Group.Children.begin(), m_Group.Children.end(),
        [this](const auto& child) { return child.strChildId == m_SelectedChild; });
    return found == m_Group.Children.end() ? nullptr : &*found;
}
EFFECT_V2_DOCUMENT* CEffectEditingSession::Selected_Document()
{
    const auto* child = Selected_Child();
    if (!child) return nullptr;
    const auto found = m_Leaves.find(child->strEffectId);
    return found == m_Leaves.end() ? nullptr : &found->second.document;
}
void CEffectEditingSession::Remove_Selected()
{
    if (m_NativeLeaf) return;
    std::erase_if(m_Group.Children, [this](const auto& row) { return row.strChildId == m_SelectedChild; });
    m_SelectedChild.clear(); Touch();
}

bool CEffectEditingSession::Save(std::string& status)
{
    if (Empty()) { status = "Create an Effect first."; return false; }
    struct WRITE { std::filesystem::path path; std::string before, after; bool existed = false; };
    std::vector<WRITE> writes;
    std::set<std::string> visited;
    // Validate all changed CPU documents before the first filesystem mutation.
    for (const auto& child : m_Group.Children)
    {
        if (!visited.insert(child.strEffectId).second) continue;
        const auto it = m_Leaves.find(child.strEffectId);
        if (it == m_Leaves.end()) { status = "Missing element draft: " + child.strEffectId; return false; }
        const auto& leaf = it->second;
        if (leaf.document.strEffectId != child.strEffectId)
        { status = "Effect draft identity changed; no source was written."; return false; }
        const auto text = CEffectV2Document::Serialize_Document(leaf.document);
        EFFECT_V2_DOCUMENT validated;
        if (!CEffectV2Document::Parse_Document(text, validated, status)) return false;
        writes.push_back({CEffectV2Document::Document_Path(child.strEffectId), leaf.baseline, text, leaf.existed});
    }
    std::string groupText;
    if (m_NativeLeaf)
    {
        if (m_Group.Children.size() != 1u || m_Group.Children.front().strEffectId != m_NativeLeafId)
        { status = "Native Effect selection is inconsistent; no source was written."; return false; }
    }
    else
    {
        groupText = CEffectV2Document::Serialize_Group(m_Group);
        EFFECT_V2_GROUP validated;
        if (!CEffectV2Document::Parse_Group(groupText, validated, status)) return false;
        writes.push_back({CEffectV2Document::Group_Path(m_Group.strGroupId), m_GroupBaseline, groupText, m_GroupExisted});
    }
    for (const auto& write : writes)
    {
        std::string current; bool exists = false;
        if (!Read(write.path, current, exists, status)) return false;
        if (exists != write.existed || current != write.before)
        { status = "Save preserved disk data: this resource changed in another editor. Reopen it before saving: " + write.path.filename().string(); return false; }
    }
    size_t committed = 0;
    for (; committed < writes.size(); ++committed)
    {
        const auto& write = writes[committed];
        if (write.existed && write.before == write.after) continue;
        if (!CEffectV2Document::Write_AtomicFile(write.path, write.after, status)) break;
    }
    if (committed != writes.size())
    {
        const auto failure = status;
        std::string rollbackErrors;
        while (committed > 0)
        {
            const auto& write = writes[--committed];
            if (write.existed && write.before == write.after) continue;
            std::string current, error; bool exists = false;
            if (!Read(write.path, current, exists, error) || !exists || current != write.after)
            { rollbackErrors += " Changed during rollback: " + write.path.generic_string(); continue; }
            if (write.existed)
            {
                if (!CEffectV2Document::Write_AtomicFile(write.path, write.before, error)) rollbackErrors += " " + error;
            }
            else
            {
                std::error_code ec;
                std::filesystem::remove(write.path, ec);
                if (ec) rollbackErrors += " " + ec.message();
            }
        }
        status = failure + rollbackErrors;
        return false;
    }
    for (const auto& id : visited)
    {
        auto& leaf = m_Leaves.at(id);
        leaf.baseline = CEffectV2Document::Serialize_Document(leaf.document); leaf.existed = true;
    }
    if (!m_NativeLeaf) { m_GroupBaseline = groupText; m_GroupExisted = true; }
    m_Dirty = false;
    CEffectV2Runtime::Invalidate_Caches();
    status = m_NativeLeaf ? "Saved native Effect; its ID and file format were retained." :
        "Saved Effect group and referenced elements. No animation or global graph validation was required.";
    return true;
}
}
