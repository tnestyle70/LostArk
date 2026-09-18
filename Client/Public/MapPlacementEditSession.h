#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#ifdef _DEBUG
#include "MapAssetCatalog.h"
#include "MapPlacementDocument.h"
#include "MapPlacementRuntime.h"
#include "MapPublishRunner.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

class CTrigger_Box;
class IMapAuthoringHost;

/* Live map placement editing for the Debug World Level Tool.

   The session borrows the placements of the Level that currently owns the
   Area (IMapAuthoringHost) exactly the way the Map Tool does when it is
   attached to a live map: the Level keeps owning every runtime container and
   the session only writes poses through CMapPlacementRuntime.

   The authoring record of a placement is kept here, apart from the live
   entry, because a self motion or a sequence samples the live entry every
   frame; Save must store the authored pose, never a sampled one. */
class CMapPlacementEditSession final
{
public:
	CMapPlacementEditSession() = default;
	~CMapPlacementEditSession();
	CMapPlacementEditSession(const CMapPlacementEditSession&) = delete;
	CMapPlacementEditSession& operator=(const CMapPlacementEditSession&) = delete;

	struct BIND_DESC final
	{
		std::string areaId;
		std::filesystem::path sourceCatalog;
		std::filesystem::path sourcePlacements;
		std::filesystem::path sourceMaterials;
		/* MapCatalog declares a light pair for this Area, so the Area publish
		   rewrites <Area>.maplights.json as well. */
		bool_t declaresLights = false;
	};

	/* Reads the authoring document of the Area and binds it to the live Level
	   that owns it. Captures the freshness baseline bytes. */
	bool_t Bind(const BIND_DESC& desc, std::string& outStatus);
	/* Drops the outline and the pick, keeps the draft in memory. */
	void End();
	/* Per frame: reports a finished publish, ends the session when the Level
	   stopped owning the Area, and keeps the outline on the selected pose.
	   outlineVisible is false while the tool window is closed or hidden; the
	   outline is then removed and rebuilt when it shows again. */
	void Update(bool_t outlineVisible);

	bool_t Is_Bound() const { return m_bBound; }
	bool_t Is_ReadOnly() const { return m_bReadOnly; }
	bool_t Is_Dirty() const { return m_bDirty; }
	bool_t Is_Publishing() const { return m_PublishRunner.Is_Running(); }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_ReadOnlyReason() const { return m_ReadOnlyReason; }
	const std::string& Get_Status() const { return m_Status; }
	void Set_Status(std::string status) { m_Status = std::move(status); }
	/* True once after the set of draft rows changed. */
	bool_t Consume_RowsDirty();

	uint64_t Get_SelectedPlacementId() const { return m_iSelectedPlacementId; }
	void Select_Placement(uint64_t placementId);
	void Clear_Selection();
	const std::vector<MAP_PLACEMENT_RECORD>& Get_Draft() const { return m_Draft; }
	const MAP_PLACEMENT_RECORD* Find_Draft(uint64_t placementId) const;
	/* Read-only: the tool labels its rows from the same Area catalog. */
	const CMapAssetCatalog& Get_Catalog() const { return m_Catalog; }
	/* Empty while the placement can take a transform edit, else the reason
	   the World tool shows next to the disabled widgets. */
	std::string Describe_EditBlock(uint64_t placementId) const;
	bool_t Is_SessionCreated(uint64_t placementId) const;

	/* One shot viewport pick. MainApp owns the click and calls back. */
	void Arm_Pick();
	void Cancel_Pick(std::string reason);
	bool_t Is_PickArmed() const { return m_bPickArmed; }
	void Complete_Pick(const float3_t& worldPoint);

	bool_t Apply_Transform(uint64_t placementId, const MAP_PLACEMENT_RECORD& staged);
	bool_t Duplicate_Selected();
	bool_t Delete_Selected();
	/* Writes the authoring document after the freshness check, keeps a
	   rollback copy and starts the Area publish. */
	bool_t Save();

private:
	struct OUTLINE final
	{
		uint64_t placementId = 0u;
		MAP_PLACEMENT_RECORD pose;
		shared_ptr<CTrigger_Box> object;
	};

	IMapAuthoringHost* Resolve_Host() const;
	MAP_RUNTIME_PLACED_ENTRY* Find_Entry(uint64_t placementId);
	const MAP_RUNTIME_PLACED_ENTRY* Find_Entry(uint64_t placementId) const;
	void Remember_Draft(const MAP_PLACEMENT_RECORD& record);
	void Forget_Draft(uint64_t placementId);
	void Reindex_Draft();
	void Rebase_SelfMotions();
	uint64_t Allocate_EditorPlacementId();
	bool_t Can_ChangeStructure();
	void Refresh_Outline();
	void Remove_Outline();
	void Poll_Publish();
	std::filesystem::path Rollback_CopyPath() const;

	bool_t m_bBound = false;
	bool_t m_bReadOnly = false;
	bool_t m_bDirty = false;
	bool_t m_bRowsDirty = false;
	bool_t m_bPickArmed = false;
	bool_t m_bRollbackValid = false;
	bool_t m_bDeclaresLights = false;
	uint32_t m_iLevelIndex = ETOUI(LEVEL::END);
	uint64_t m_iSelectedPlacementId = 0u;
	uint64_t m_iNextPlacementId = 1u;
	std::string m_AreaId;
	std::string m_ReadOnlyReason;
	std::string m_Status;
	std::string m_BaselineBytes;
	std::string m_RollbackBytes;
	std::filesystem::path m_SourcePlacements;
	/* The source catalog of the Area with the live Level's prototypes bound
	   onto it, so one catalog serves both the document writer and the runtime
	   clone. */
	CMapAssetCatalog m_Catalog;
	std::vector<MAP_PLACEMENT_RECORD> m_Draft;
	std::unordered_map<uint64_t, size_t> m_DraftIndex;
	std::vector<uint64_t> m_SessionCreated;
	std::unordered_map<std::string, shared_ptr<Engine::CModel>> m_ModelCache;
	OUTLINE m_Outline;
	CMapPublishRunner m_PublishRunner;
};

NS_END
#endif
