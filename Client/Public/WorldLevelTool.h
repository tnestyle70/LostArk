#pragma once

#include "KoukuSaydonCompositionDocument.h"
#include <array>
#include <filesystem>
#include <optional>

#ifdef _DEBUG
namespace Client
{
enum class WORLD_LEVEL_COMPOSITION_OWNER { ACTION, SEQUENCE };
enum class WORLD_LEVEL_REQUEST_KIND { OPEN_MAP, OPEN_WORLD_OBJECT, OPEN_COMPOSITION, OPEN_LIGHT, FOCUS };

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
};

// Read-only projection of the existing authoring owners. Requests carry stable
// IDs; opening an owner must preserve its in-memory draft and its save contract.
class CWorldLevelTool final
{
public:
    void Open(const std::string& activeAreaId);
    void Set_ActiveArea(const std::string& areaId);
    void Set_CompositionView(WORLD_LEVEL_COMPOSITION_OWNER owner,
        const KOUKU_SAYDON_COMPOSITION_DOCUMENT* document, uint64_t generation);
    void Render();
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
        std::string key, name, kind, assetId, anchor, status;
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
    bool Load_Areas();
    bool Refresh();
    void Rebuild_Rows();
    void Append_CompositionRows(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        WORLD_LEVEL_COMPOSITION_OWNER owner, bool draft);
    void Request_Edit(const ROW& row);
    void Request_Focus(const ROW& row);

    bool m_Open = false;
    bool m_InteractionRequested = false;
    bool m_RowsDirty = true;
    bool m_FilterDirty = true;
    std::string m_ActiveAreaId, m_SelectedAreaId, m_SelectedKey, m_Status;
    std::vector<AREA> m_Areas;
    std::vector<ROW> m_SavedRows, m_Rows;
    std::vector<size_t> m_FilteredRows;
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
