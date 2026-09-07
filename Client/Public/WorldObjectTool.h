#pragma once

#include "WorldSequenceDocument.h"
#include "PhysicalResourceCatalog.h"
#include "CompositionResourceTree.h"

#include <array>
#include <filesystem>

#ifdef _DEBUG
namespace Client
{
class CLevel_KakulSaydonArena;

class CWorldObjectTool final
{
public:
    ~CWorldObjectTool();
    void Open();
    void Deactivate();
    void Update(f32_t seconds, bool_t active);
    void Render();
    bool_t Is_Open() const { return m_Open; }
    bool Consume_InteractionRequest();
    // Only a saved document may become a Workbench resource inventory.
    const CWorldSequenceDocument* Get_SavedDocument() const { return m_Ready ? &m_SavedDocument : nullptr; }
    uint64_t Get_SavedGeneration() const { return m_SavedGeneration; }

private:
    bool Load_Source();
    bool Save_Source();
    bool Matches_SourceBaseline();
    void Start_Publish();
    void Poll_Publish();
    void Mark_Dirty();
    void Stop_Preview();
    bool Begin_Preview();
    void Seek(f32_t clockMs);
    f32_t SpanMs() const;
    void Select_Object(const std::string& id);
    void Select_State(const std::string& id);
    std::vector<std::string> StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const;
    bool Create_Object();
    void Create_State();
    bool Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& stateName,
        WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance);
    void Change_ResourceAnchor(WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind);
    void Render_WindowMenu();
    void Render_Toolbar();
    void Render_Resources();
    void Render_Detail();
    void Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_PhysicalResources();
    void Rebuild_PhysicalTree();

    bool m_Open = false;
    bool m_ResourcesOpen = true;
    bool m_SequencerOpen = true;
    bool m_DetailOpen = true;
    bool m_ResetLayoutRequested = false;
    bool m_InteractionRequested = false;
    bool m_PreviewAtCharacter = true;
    bool m_Ready = false;
    bool m_Dirty = false;
    bool m_PreviewActive = false;
    bool m_PreviewDirty = false;
    bool m_Playing = false;
    bool m_Loop = false;
    CLevel_KakulSaydonArena* m_PreviewLevel = nullptr;
    f32_t m_ClockMs = 0.f;
    f32_t m_Zoom = 100.f;
    CWorldSequenceDocument m_Document;
    CWorldSequenceDocument m_SavedDocument;
    WORLD_SEQUENCE_PLACEMENT_MAP m_MapTargets;
    WORLD_SEQUENCE_DEPLOY_MAP m_DeployTargets;
    std::filesystem::path m_SourcePath;
    std::filesystem::path m_PlacementPath;
    std::filesystem::path m_DeployPath;
    std::string m_SourceBytes;
    std::string m_PlacementBytes;
    std::string m_DeployBytes;
    std::string m_Status;
    std::string m_PreviewStatus;
    std::string m_SelectedObject;
    std::string m_SelectedInstance;
    size_t m_SelectedTrack = 0;
    size_t m_SelectedKey = 0;
    uint64_t m_SavedGeneration = 0;
    std::array<char, 128> m_NewObjectName{};
    int m_NewObjectAnchor = 0;
    bool m_CreateObjectFailed = false;
    std::array<char, 128> m_NewStateName{};
    std::array<char, 256> m_ObjectSearch{};
    std::array<char, 256> m_PhysicalSearch{};
    std::vector<PHYSICAL_RESOURCE_ASSET> m_PhysicalAssets;
    COMPOSITION_RESOURCE_TREE_NODE m_PhysicalTree;
    std::string m_PhysicalStatus;
    std::string m_SelectedPhysical;
    bool m_PhysicalScanned = false;
    int m_PhysicalSlot = 0;
    HANDLE m_PublishProcess = nullptr;
    std::filesystem::path m_PublishLog;
};
}
#endif
