#pragma once

#include "EffectV2_Document.h"
#include <map>
#include <string>

namespace Client
{
// A CPU document draft. Selecting, tuning and saving never require a GPU
// preview, an animation target, or admission of another boss's effect graph.
class CEffectEditingSession final
{
public:
    struct LEAF final
    {
        EFFECT_V2_DOCUMENT document;
        std::string baseline;
        bool existed = false;
    };
    void Create(const std::string& name, EFFECT_V2_TYPE type,
        EFFECT_V2_RESOURCE_KIND kind = EFFECT_V2_RESOURCE_KIND::GROUP);
    bool Promote_ToGroup(std::string& status);
    EFFECT_V2_RESOURCE_KIND Resource_Kind() const { return m_NativeLeaf ? EFFECT_V2_RESOURCE_KIND::LEAF : EFFECT_V2_RESOURCE_KIND::GROUP; }
    const std::string& Resource_Id() const { return m_NativeLeaf ? m_NativeLeafId : m_Group.strGroupId; }
    bool Load(EFFECT_V2_RESOURCE_KIND kind, const std::string& id, std::string& status);
    bool Append(const std::string& id, std::string& status);
    void Add_Element(EFFECT_V2_TYPE type, const std::string& name);
    void Remove_Selected();
    bool Save(std::string& status);
    bool Revert(std::string& status);
    void Reset();
    void Touch() { m_Dirty = true; ++m_EditGeneration; }
    uint64_t Edit_Generation() const { return m_EditGeneration; }
    bool Dirty() const { return m_Dirty; }
    bool Empty() const { return m_Group.strGroupId.empty(); }
    EFFECT_V2_GROUP& Group() { return m_Group; }
    const EFFECT_V2_GROUP& Group() const { return m_Group; }
    const std::map<std::string, LEAF>& Leaves() const { return m_Leaves; }
    EFFECT_V2_DOCUMENT* Selected_Document();
    EFFECT_V2_GROUP_CHILD* Selected_Child();
    void Select(const std::string& childId) { m_SelectedChild = childId; }
    const std::string& Selection() const { return m_SelectedChild; }
    static std::string New_Id(const char* prefix);
private:
    EFFECT_V2_GROUP m_Group;
    std::map<std::string, LEAF> m_Leaves;
    std::string m_SelectedChild, m_GroupBaseline;
    uint64_t m_EditGeneration = 0u;
    bool m_GroupExisted = false, m_Dirty = false, m_NativeLeaf = false;
    std::string m_NativeLeafId;
};
}
