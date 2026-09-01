#pragma once

#include "Client_Defines.h"
#include "MapPlacementDocument.h"
#include "MapPlacementRuntime.h"
#include "DeployPropRuntime.h"
#include "WorldSequenceDocument.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CMapAssetCatalog;

class CWorldSequenceToolPanel final
{
private:
	struct PREVIEW_TARGET
	{
		uint64_t placementId = 0;
		MAP_PLACEMENT_RECORD baseline;
		bool_t runtimeVisible = true;
	};

	struct PREVIEW_DEPLOY_TARGET
	{
		uint64_t runtimePlacementId = 0;
	};

public:
	bool_t Load_Area(
		const std::filesystem::path& path,
		const std::filesystem::path& placementPath,
		const std::string& areaId,
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_PLACEMENT_RECORD>& placements,
		const CDeployPropRuntime& deployRuntime,
		std::string& outStatus);
	bool_t Save(
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		const CDeployPropRuntime& deployRuntime,
		std::string& outStatus);
	bool_t Validate(
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		const CDeployPropRuntime& deployRuntime,
		std::string& outStatus) const;
	bool_t Matches_LinkedSourceBaseline(std::string& outStatus) const;
	void Adopt_VerifiedLinkedSourceBaseline(
		CWorldSequenceToolPanel& verified) noexcept;
	bool_t Has_SameDocument(const CWorldSequenceToolPanel& other) const
	{
		return m_Document.Is_Equivalent(other.m_Document);
	}

	void Update(
		f32_t timeDelta,
		bool_t isPanelActive,
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		CDeployPropRuntime& deployRuntime);
	void Render(
		bool_t isMapAuthoringLevel,
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		uint64_t& selectedPlacementId,
		CDeployPropRuntime& deployRuntime);
	void Stop_AndRestore(
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		CDeployPropRuntime& deployRuntime);
	void Reset();

	/* Stable instance IDs an authored trigger may start, in document order. */
	std::vector<std::string> Get_InstanceIds() const;

	bool_t Is_Dirty() const noexcept { return m_bDirty; }
	bool_t Is_Ready() const noexcept { return !m_Document.Get_AreaId().empty(); }
	bool_t Is_PreviewActive() const noexcept { return m_bPreviewActive; }
	const std::string& Get_Status() const noexcept { return m_Status; }
	void Report_SaveAllResult(bool_t succeeded, const std::string& status)
	{
		m_Status = succeeded ? "Save All completed: " + status :
			"Save All failed: " + status;
	}
	void Report_ReloadAllResult(bool_t succeeded, const std::string& status)
	{
		m_Status = succeeded ? "Reload All completed: " + status :
			"Reload All failed: " + status;
	}
	void Restore_DirtyAfterFailedTransaction() noexcept
	{
		m_bDirty = true;
	}
	bool_t Consume_SaveAllRequest() noexcept
	{
		const bool_t requested = m_bSaveAllRequested;
		m_bSaveAllRequested = false;
		return requested;
	}
	bool_t Consume_ReloadAllRequest() noexcept
	{
		const bool_t requested = m_bReloadAllRequested;
		m_bReloadAllRequested = false;
		return requested;
	}

private:
	static WORLD_SEQUENCE_PLACEMENT_MAP Collect_Placements(
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_PLACEMENT_RECORD>& placements);
	static WORLD_SEQUENCE_PLACEMENT_MAP Collect_Placements(
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements);
	static WORLD_SEQUENCE_DEPLOY_MAP Collect_DeployPlacements(
		const CDeployPropRuntime& deployRuntime);
	static bool_t Try_ParseTargetId(
		const WORLD_SEQUENCE_BINDING& binding,
		uint64_t& outTargetId);
	static MAP_RUNTIME_PLACED_ENTRY* Find_Placement(
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		uint64_t placementId);
	static const WORLD_SEQUENCE_TRACK* Find_Track(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const std::string& slotId);
	static WORLD_SEQUENCE_TRANSFORM_KEY Sample_Track(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const WORLD_SEQUENCE_TRACK& track,
		f32_t timeMs);

	bool_t Begin_Preview(
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		CDeployPropRuntime& deployRuntime);
	bool_t Apply_Preview(
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		CDeployPropRuntime& deployRuntime);
	bool_t Apply_RuntimeRecord(
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		MAP_RUNTIME_PLACED_ENTRY& entry,
		const MAP_PLACEMENT_RECORD& record);

	void Create_Template();
	void Create_AnimationTemplate(CDeployPropRuntime& deployRuntime);
	void Duplicate_SelectedTemplate();
	void Delete_SelectedTemplate();
	void Apply_SelectedTemplate(
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		uint64_t placementId);
	void Apply_SelectedAnimationTemplate(CDeployPropRuntime& deployRuntime);
	void Delete_SelectedInstance();
	void Mark_Dirty();
	std::string Allocate_TemplateId() const;
	std::string Allocate_InstanceId(const std::string& templateId) const;

	void Render_DocumentToolbar(
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		CDeployPropRuntime& deployRuntime);
	void Render_TemplateList();
	void Render_InstanceList();
	void Render_ObjectBrowser(
		const CMapAssetCatalog& catalog,
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		uint64_t& selectedPlacementId);
	void Render_DeployBrowser(CDeployPropRuntime& deployRuntime);
	void Render_TemplateEditor(
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		uint64_t selectedPlacementId,
		CDeployPropRuntime& deployRuntime);
	void Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence);
	void Render_AnimationEditor(
		WORLD_SEQUENCE_TEMPLATE& sequence,
		CDeployPropRuntime& deployRuntime);
	void Render_Playback(
		uint32_t authoringLevelIndex,
		const CMapAssetCatalog& catalog,
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		CDeployPropRuntime& deployRuntime);

private:
	CWorldSequenceDocument m_Document;
	std::filesystem::path m_Path;
	std::filesystem::path m_PlacementPath;
	std::string m_PlacementBaselineBytes;
	std::string m_SequenceBaselineBytes;
	bool_t m_bSequenceBaselineExists = false;
	bool_t m_bDirty = false;
	bool_t m_bSaveAllRequested = false;
	bool_t m_bReloadAllRequested = false;
	std::string m_Status = "Select an Area to edit world sequences";
	std::string m_SelectedTemplateId;
	std::string m_SelectedInstanceId;
	size_t m_iSelectedTrack = 0;
	size_t m_iSelectedAnimationTrack = 0;
	size_t m_iSelectedKey = 0;
	char_t m_TemplateFilter[128]{};
	char_t m_InstanceFilter[128]{};
	char_t m_ObjectFilter[128]{};
	std::string m_CachedObjectFilter;
	size_t m_iCachedObjectPlacementCount = SIZE_MAX;
	uint64_t m_iCachedObjectFingerprint = 0;
	std::vector<size_t> m_FilteredObjectIndices;
	std::string m_RotationEditorKey;
	float3_t m_RotationEditorDegrees = {};
	uint64_t m_iSelectedDeployPlacementId = 0;

	bool_t m_bPreviewActive = false;
	bool_t m_bPlaying = false;
	bool_t m_bPreviewNeedsRefresh = false;
	bool_t m_bLoop = true;
	f32_t m_fPreviewTimeMs = 0.f;
	std::vector<PREVIEW_TARGET> m_PreviewTargets;
	std::vector<PREVIEW_DEPLOY_TARGET> m_PreviewDeployTargets;
	std::unordered_map<std::string, shared_ptr<Engine::CModel>> m_PreviewModels;
};

NS_END
