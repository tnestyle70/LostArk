#pragma once

#include "KoukuSaydonCompositionDocument.h"
#include <array>
#include <filesystem>
#include <optional>
#include <chrono>

#ifdef _DEBUG
#include "MapStaticChunkObject.h"
#include "WorldLevelHierarchy.h"
#include <unordered_map>
#include <unordered_set>
namespace Client
{
enum class WORLD_LEVEL_COMPOSITION_OWNER { ACTION, SEQUENCE };
/* Scene requests use the live World Scene inspector; this inventory owns no
   second placement draft or GPU world-point selection path. */
enum class WORLD_LEVEL_REQUEST_KIND { OPEN_MAP, OPEN_WORLD_OBJECT, OPEN_COMPOSITION, OPEN_LIGHT, FOCUS, PICK_IN_SCENE, OPEN_GUIDE, SET_CHUNK_MODE, INSPECT_PLACEMENT, INSPECT_METADATA };

struct WORLD_LEVEL_TOOL_REQUEST final
{
    WORLD_LEVEL_REQUEST_KIND kind = WORLD_LEVEL_REQUEST_KIND::OPEN_MAP;
    std::string areaId;
    uint64_t placementId = 0u;
    bool deploy = false;
    std::string sequenceInstanceId;
    std::string objectId;
    std::string sourceItemId;
    std::string patternId;
    std::string occurrenceId;
    WORLD_LEVEL_COMPOSITION_OWNER compositionOwner = WORLD_LEVEL_COMPOSITION_OWNER::ACTION;
    float3_t position{};
    float focusRadius = 8.f;
    bool chunkEnabled = true, chunkHlodEnabled = true;
    uint64_t runtimeGeneration = 0u;
};

// Read-only projection of the existing authoring owners. Requests carry stable
// IDs; opening an owner must preserve its in-memory draft and its save contract.
class CWorldLevelTool final
{
public:
    CWorldLevelTool();
    ~CWorldLevelTool();
    CWorldLevelTool(const CWorldLevelTool&) = delete;
    CWorldLevelTool& operator=(const CWorldLevelTool&) = delete;

    void Open(const std::string& activeAreaId);
    void Set_ActiveArea(const std::string& areaId);
    const std::string& Get_SelectedAreaId() const { return m_SelectedAreaId; }
    void Sync_LiveSelection(const std::string& areaId, uint64_t placementId, bool deploy, bool force = false);
    void Hide_MetadataDetails() { m_ShowMetadata = false; }
    void Set_CompositionView(WORLD_LEVEL_COMPOSITION_OWNER owner,
        const KOUKU_SAYDON_COMPOSITION_DOCUMENT* document, uint64_t generation);
    void Render();
    void Render_MetadataDetails();
    bool Needs_ChunkView(const std::string& area, const void* source, uint64_t generation) const;
    void Set_ChunkView(std::string area, const void* source, uint64_t generation,
        std::vector<MAP_CHUNK_DEBUG_ROW> rows, bool enabled, bool hlodEnabled);
    const std::vector<MAP_CHUNK_DEBUG_ROW>& Get_ChunkView() const { return m_ChunkRows; }
    bool Show_ChunkBounds() const
    { return m_Open && m_ShowChunkBounds && m_ChunkSource && m_ChunkArea == m_ActiveAreaId && m_ChunkArea == m_SelectedAreaId; }
    bool Show_AllChunkBounds() const { return m_ShowAllChunkBounds || m_SelectedChunk == UINT32_MAX; }
    uint32_t Get_SelectedChunk() const { return m_SelectedChunk; }
    bool Is_Open() const { return m_Open; }
    bool Consume_InteractionRequest();
    bool Consume_Request(WORLD_LEVEL_TOOL_REQUEST& request);
    void Set_Status(std::string status) { m_Status = std::move(status); }

private:
    struct AREA final
    {
        std::string id;
        std::string label;
        std::filesystem::path catalog, placements, materials, deployCatalog, deployPlacements;
        std::filesystem::path sequences, gameplay, lights, effects;
    };
    struct ROW final
    {
        std::string key, name, kind, assetId, anchor, status, sourceLevel;
        WORLD_LEVEL_TOOL_REQUEST target;
        bool hasPosition = false;
        bool canEdit = true;
        bool visible = true;
        uint32_t startMs = 0u, durationMs = 0u;
        bool timed = false;
    };
    struct COMPOSITION_VIEW final
    {
        std::optional<KOUKU_SAYDON_COMPOSITION_DOCUMENT> document;
        const KOUKU_SAYDON_COMPOSITION_DOCUMENT* source = nullptr;
        uint64_t generation = 0u;
    };
    // These nodes only organize the inventory. They never own runtime transforms.
    struct TREE_NODE final
    {
        std::string id, label, folderId;
        size_t parent = SIZE_MAX, row = SIZE_MAX;
        std::vector<size_t> children;
        bool matches = true, branchMatches = true;
    };
    struct VISIBLE_NODE final { size_t node = 0u; uint32_t depth = 0u; };
    enum class PARENT_ACTION { NONE, CREATE, RENAME, MOVE, REMOVE };
    void Render_ChunkView();
    void Render_Tree();
    void Render_HierarchyControls();
    void Remember_HierarchySelection();
    bool Apply_HierarchyHistory(bool redo);
    void Render_ParentDialog();
    void Rebuild_Tree();
    void Rebuild_VisibleTree();
    void Select_Node(size_t node, bool control, bool shift);
    void Reveal_Node(size_t node);
    bool Move_Selection(const std::string& parent, const std::string& newName = {});
    void Begin_ParentAction(PARENT_ACTION action);
    bool Row_Matches(const ROW& row, const std::string& search) const;
    const ROW* Selected_Row() const;
    bool Load_Areas();
    bool Refresh();
    void Rebuild_Rows();
    void Append_CompositionRows(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        WORLD_LEVEL_COMPOSITION_OWNER owner, bool draft);
    void Request_Edit(const ROW& row);
    void Request_Focus(const ROW& row);

    std::vector<MAP_CHUNK_DEBUG_ROW> m_ChunkRows;
    std::string m_ChunkArea;
    const void* m_ChunkSource = nullptr;
    uint64_t m_ChunkGeneration = 0u;
    std::chrono::steady_clock::time_point m_ChunkNextRefresh{};
    uint32_t m_SelectedChunk = UINT32_MAX;
    bool m_ShowChunkBounds = false, m_ShowAllChunkBounds = false;
    bool m_ChunkEnabled = true, m_ChunkHlodEnabled = true;
    bool m_Open = false;
    bool m_InteractionRequested = false;
    bool m_RowsDirty = true;
    bool m_FilterDirty = true, m_TreeDirty = true, m_VisibleDirty = true;
    std::string m_ActiveAreaId, m_SelectedAreaId, m_SelectedKey, m_Status;
    std::vector<AREA> m_Areas;
    std::vector<ROW> m_SavedRows, m_Rows;
    CWorldLevelHierarchy m_Hierarchy;
    std::vector<TREE_NODE> m_Nodes;
    std::vector<VISIBLE_NODE> m_VisibleNodes;
    std::unordered_map<std::string, size_t> m_NodeIndex, m_RowIndex;
    std::unordered_set<std::string> m_ExpandedNodes, m_SelectedNodes;
    std::string m_SelectedNode, m_SelectionAnchor, m_ScrollToNode;
    std::string m_LastLiveArea, m_LastLiveKey;
    bool m_ShowMetadata = false, m_MetadataOpen = false;
    PARENT_ACTION m_ParentAction = PARENT_ACTION::NONE;
    std::array<char, 256u> m_ParentName{};
    std::string m_ParentDestination, m_ParentDialogFolder;
    std::unordered_set<std::string> m_ParentSelection;
    uint64_t m_DragToken = 0u;
    std::unordered_set<std::string> m_DragSelection;
    bool m_ParentDialogRequested = false;
    std::string m_FilterSearch;
    int m_FilterKind = -1;
    std::array<COMPOSITION_VIEW, 2u> m_Compositions;
    std::array<std::optional<KOUKU_SAYDON_COMPOSITION_DOCUMENT>, 2u> m_SavedCompositions;
    std::array<char, 256u> m_Search{};
    int m_KindFilter = 0;
    std::optional<WORLD_LEVEL_TOOL_REQUEST> m_Request;
};
}
#endif
