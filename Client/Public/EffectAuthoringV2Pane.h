#pragma once

#include "EffectEditingSession.h"
#include "EffectResourceCatalog.h"
#include "EffectV2_Catalog.h"
#include "EffectCompositionWorldResource.h"

namespace Client
{
class CEffect_Tool_V2;

// CPU document panels owned by the independent Effect Tool V2.
// The pane owns CPU drafts; the shared resource editor owns slot thumbnails.
class CEffectAuthoringV2Pane final
{
public:
    explicit CEffectAuthoringV2Pane(CEffect_Tool_V2& editor);
    bool Open(const EFFECT_RESOURCE_KEY& key);
    bool Create(const std::string& name, EFFECT_V2_TYPE type,
        EFFECT_RESOURCE_OWNER_KIND owner = EFFECT_RESOURCE_OWNER_KIND::V2_LEAF);
    bool Append(const EFFECT_RESOURCE_KEY& key);
    void Render_ToolContents();
    void Render_DetailContents();
    void Render_ResourceContents();
    bool Snapshot(EFFECT_RESOURCE_KEY& outKey,
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& outSnapshot, std::string& error) const;
    bool Consume_Saved(EFFECT_RESOURCE_KEY& outKey, std::string& outName);
    bool Consume_Play(EFFECT_RESOURCE_KEY& outKey, std::string& outChildId);
    EFFECT_RESOURCE_KEY Current_Key() const;
    bool Owns_Resource(const EFFECT_RESOURCE_KEY& key) const
    { return Current_Key() == key || (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_LEAF && m_Edit.Leaves().contains(key.strStableId)); }
    uint32_t DurationMs() const;
    uint64_t Edit_Generation() const { return m_Edit.Edit_Generation(); }
    bool Dirty() const { return m_Edit.Dirty(); }
    const std::string& Status() const { return m_Status; }

private:
    void Refresh_WorldObjects();
    void Render_WorldObjects();
    bool Extend_GroupToChild(const EFFECT_V2_DOCUMENT& document,
        const EFFECT_V2_GROUP_CHILD& child, uint32_t& outDuration, std::string& error) const;

    CEffect_Tool_V2& m_Editor;
    CEffectEditingSession m_Edit;
    std::vector<EFFECT_COMPOSITION_WORLD_RESOURCE> m_WorldObjects;
    uint32_t m_WorldRevision = 0u;
    bool m_WorldLoaded = false, m_HasSaved = false, m_HasPlay = false;
    EFFECT_RESOURCE_KEY m_PlayKey;
    std::string m_PlayChildId;
    EFFECT_RESOURCE_KEY m_SavedKey;
    std::string m_SavedName, m_Status, m_WorldStatus;
    int m_AddType = 2;
    char m_ElementName[257] = {};
};
}
