#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapAssetPreview.h"
#include "MapPlacementDocument.h"
#include "MapPlacementRuntime.h"
#include "DeployPropRuntime.h"
#include "DestructionSimulationDocument.h"
#include "EncounterPatternReference.h"
#include "WorldDestructionDocument.h"
#include "NavGridBaker.h"
#include "NavGridPaintDocument.h"
#include "NavRuntimeBlockerDocument.h"
#include "WorldGameplayDocument.h"
#include "SpawnGroupDocument.h"

#include <memory>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;
class CMapStaticBatchObject;
class CTrigger_Box;
class CNpc;
class CCamera_Free;
class CDestructionSimulationController;
class CMapTool final
{
private:
	enum class TOOL_MODE
	{
		MAP_ASSETS,
		WORLD_GAMEPLAY,
		WORLD_DESTRUCTION,
		NAVIGATION,
		CAMERA,
	};

	enum class EDITOR_NAVIGATION_POLICY
	{
		NONE,
		SOURCE_PAINT,
		SOURCE_PAINT_BLOCKERS,
	};

	enum class EDITOR_GAMEPLAY_POLICY
	{
		NONE,
		REQUIRED,
	};

	struct EDITOR_AREA_DESCRIPTOR
	{
		std::string areaId;
		std::string label;
		std::filesystem::path sourceCatalog;
		std::filesystem::path sourcePlacements;
		std::filesystem::path sourceDeployCatalog;
		std::filesystem::path sourceDeployPlacements;
		std::filesystem::path navigationSource;
		std::filesystem::path navigationPaint;
		std::filesystem::path navigationBlockers;
		std::filesystem::path gameplayDocument;
		std::filesystem::path encounterReference;
		std::filesystem::path worldEventsDocument;
		std::filesystem::path destructionSimulationDocument;
		EDITOR_NAVIGATION_POLICY navigationPolicy =
			EDITOR_NAVIGATION_POLICY::NONE;
		EDITOR_GAMEPLAY_POLICY gameplayPolicy =
			EDITOR_GAMEPLAY_POLICY::NONE;
		bool_t allowNavigationBootstrap = false;
	};

	enum class NAVIGATION_MODE
	{
		BAKE,
		WALKABILITY,
		DESTRUCTION_AREA,
	};

	enum class NAVIGATION_EDIT_ACTION
	{
		APPLY,
		FORCE_WALKABLE,
		ERASE,
	};
	//bake ���̵� �޽�
	enum class NAV_BOUNDS_STATE
	{
		IDLE,
		PLACING,
	};

	struct NAVIGATION_RENDER_RESOURCES;

	enum class PLACEMENT_STATE
	{
		IDLE,
		ARMED,
	};

	using PLACED_ENTRY = MAP_RUNTIME_PLACED_ENTRY;
	using STATIC_BATCH_ENTRY = MAP_RUNTIME_STATIC_BATCH_ENTRY;

	enum class ENVIRONMENT_PHASE
	{
		BASELINE,
		SPACEHOLE,
		CHAOS_GATE,
	};

	struct TRIGGER_BOX_ENTRY
	{
		std::string placementId;
		shared_ptr<CTrigger_Box> object;
	};

	struct NPC_PREVIEW_ENTRY
	{
		std::string placementId;
		std::string archetypeId;
		shared_ptr<CNpc> object;
	};

public:
	CMapTool();
	~CMapTool();

	HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	void Toggle();
	void SetOpen(bool_t isOpen);
	void Update(f32_t fTimeDelta);
	void Render();

	bool IsOpen() const;
	bool_t ConsumesWorldLeftMouse() const;

private:
	/* Frame Update */
	void Update_DestructionSimulation(
		f32_t fTimeDelta,
		bool_t isMapAuthoringLevel);
	void Update_WorldInteraction(bool_t isAssetTest);
	void Handle_LevelTransition(
		uint32_t currentLevelIndex,
		bool_t isMapAuthoringLevel);

	/* Frame Render */
	void Render_WorldOverlay(bool_t isAssetTest);
	void Render_WorkspaceBar(bool_t isAssetTest);
	void Render_ActiveMode(bool_t isAssetTest);
	void Render_MapAssetsPanel(bool_t isAssetTest);
	void Render_WorldGameplayPanel(bool_t isAssetTest);
	void Render_SpawnGroupsPanel();
	void Render_WorldDestructionPanel(bool_t isAssetTest);
	void Render_DestructionSimpleEditor();
	void Render_DestructionSimpleWallList();
	void Render_DestructionSimpleInspector();
	void Render_DestructionSimpleTimeline();
	void Render_DestructionSimulationWindow(bool_t isAssetTest);
	void Render_DestructionSimulationOutliner();
	void Render_DestructionSimulationDetail();
	void Render_DestructionSimulationTimeline();
	void Render_DestructionEncounterSource();
	void Render_DestructionDeployList();
	void Render_DestructionWorldRows();
	void Render_DestructionNavigationRegions();
	void Render_DestructionGroupEditor();
	void Render_DestructionBindingEditor();
	void Render_DestructionTimeline();
	void Render_DestructionDiagnostics();
	void Render_ModeBar();
	void Render_CameraPanel();
	void Render_NavigationPanel();
	void Render_DestructionAreaControls();
	void Render_NavigationDiagnostics();
	void Render_NavigationOverlay();
	void Render_NavigationBakeControls();
	void Render_NavigationBoundsOverlay();
	void Render_Toolbar();
	void Render_Palette(f32_t childHeight);
	void Render_Hierarchy(f32_t childHeight);
	void Render_Inspector();
	void Render_AssetPreview();
	void Render_DecoderReport() const;

	/* Camera Runtime */
	bool_t Find_AssetTestCamera();
	bool_t Focus_ActiveEditorAreaCamera();

	/* Navigation Document and Runtime */
	bool_t Load_NavigationDocument();
	bool_t Load_RuntimeBlockers();
	bool_t Register_RuntimeBlockers();
	bool_t Set_NavigationCondition(
		const std::string& conditionId,
		bool_t value);
	bool_t Save_Navigation();
	bool_t Try_PickNavigationCell(
		int32_t& outCellX,
		int32_t& outCellZ) const;
	bool_t Try_PaintNavigation();
	bool_t Try_PlaceNavigationBounds();
	bool_t Bake_Navigation();
	bool_t Collect_NavigationBakePlacements(
		std::vector<NAVGRID_BAKE_PLACEMENT>& outPlacements,
		std::string& outStatus) const;
	bool_t Is_CellInsideNavigationBounds(
		f32_t worldX,
		f32_t worldZ) const;
	static bool_t Is_ValidNavigationBakeDesc(
		const NAVGRID_BAKE_DESC& desc);

	/* Map Asset Document and Runtime */
	bool_t Ensure_AuthoringPrototypes();
	bool_t Ensure_AuthoringPrototypes(const CMapAssetCatalog& catalog);
	bool_t Ensure_DeployAuthoringPrototypes(
		const CDeployPropCatalog& catalog);
	bool_t Ensure_DestructionDebrisAuthoringPrototypes();
	bool_t Load_EditorAreaRegistry();
	bool_t Switch_EditorArea(size_t descriptorIndex);
	bool_t Save_AllAuthoring();
	bool_t Has_UnsavedAuthoring() const;
	const EDITOR_AREA_DESCRIPTOR* Get_ActiveEditorArea() const;
	bool_t Try_PickPlacementPosition(float3_t& outPosition) const;
	bool_t Try_PlaceSelected();
	bool_t Create_Placement(const MAP_PLACEMENT_RECORD& record,
		PLACED_ENTRY& outEntry);
	bool_t Stage_PlacementRuntime(
		const vector<MAP_PLACEMENT_RECORD>& records,
		vector<PLACED_ENTRY>& outPlacements,
		vector<STATIC_BATCH_ENTRY>& outBatches);
	void Remove_PlacementRuntime(
		vector<PLACED_ENTRY>& placements,
		vector<STATIC_BATCH_ENTRY>& batches);
	static bool_t Set_RuntimeVisible(
		PLACED_ENTRY& entry, bool_t visible);
	bool_t Remove_Placement(uint64_t placementId);
	void Remove_AllPlacements();
	bool_t Save_Placements();
	bool_t Load_Placements();
	bool_t Load_DeployProps();
	bool_t Stage_DeployProps(
		const EDITOR_AREA_DESCRIPTOR& descriptor,
		CDeployPropRuntime& outRuntime);
	void Remove_DeployProps();
	void Set_DeployPhase(DEPLOY_PROP_STATE state);
	void Set_EnvironmentPhase(ENVIRONMENT_PHASE phase);
	uint64_t Allocate_EditorPlacementId();
	std::wstring Make_LayerTag(const std::string& sourceLevel) const;
	void Select_Asset(const MAP_ASSET_ENTRY& asset);
	void Arm_SelectedAsset();

	/* World Gameplay Authoring */
	bool_t Load_WorldGameplay();
	bool_t Save_WorldGameplay();
	bool_t Try_PlaceWorldGameplay();
	bool_t Try_PickWorldTriggerTarget();
	bool_t Try_PlaceSpawnAnchor();
	bool_t Load_SpawnGroups();
	bool_t Save_SpawnGroups();
	bool_t Stage_SpawnAnchorBoxes(
		const CSpawnGroupDocument& document,
		vector<TRIGGER_BOX_ENTRY>& outEntries);
	bool_t Stage_WorldTriggerBoxes(
		const CWorldGameplayDocument& document,
		vector<TRIGGER_BOX_ENTRY>& outEntries);
	void Remove_WorldTriggerBoxes(vector<TRIGGER_BOX_ENTRY>& entries);
	bool_t Stage_WorldNpcPreviews(
		const CWorldGameplayDocument& document,
		vector<NPC_PREVIEW_ENTRY>& outEntries);
	void Remove_WorldNpcPreviews(vector<NPC_PREVIEW_ENTRY>& entries);
	void Sync_WorldNpcPreviews();
	void Update_WorldTriggerBoxPresentation(bool_t isVisible);
	std::filesystem::path Get_WorldGameplayPath() const;
	std::filesystem::path Get_SpawnGroupsPath() const;

	/* World Destruction Authoring */
	bool_t Load_EncounterReference();
	bool_t Reload_DestructionAuthoring();
	bool_t Load_WorldDestruction();
	bool_t Save_WorldDestruction();
	bool_t Save_DestructionAuthoringPair();
	std::filesystem::path Get_WorldDestructionPath() const;
	bool_t Try_PickDeployProp(
		uint64_t& outRuntimePlacementId,
		std::string& outFailure) const;
	bool_t Select_DestructionWall(
		uint64_t runtimePlacementId,
		const char_t* source);
	void Sync_DestructionDraftFromSelection();
	void Load_DestructionDraftFromBinding(
		const DESTRUCTION_BINDING& binding);
	const ENCOUNTER_STAGE_REFERENCE* Find_SelectedDestructionStage() const;
	bool_t Apply_SimpleDestructionAuthoring();
	bool_t Validate_DestructionExternalReferences(
		const CWorldDestructionDocument& destruction,
		const CDeployPropRuntime& deployRuntime,
		const CNavRuntimeBlockerDocument& blockers,
		const CWorldGameplayDocument& worldGameplay,
		const CEncounterPatternReference& encounter,
		std::string& outStatus) const;
	bool_t Validate_CurrentDestructionReferences(
		std::string& outStatus) const;
	void Use_DestructionTimelineTime();
	bool_t Load_DestructionSimulation();
	bool_t Save_DestructionSimulation();
	std::filesystem::path Get_DestructionSimulationPath() const;
	bool_t Create_DefaultDestructionSimulationProfile();
	bool_t Modify_DestructionGroupMember(
		uint64_t placementId,
		bool_t addMember);
	bool_t Request_StageDestructionSimulation(
		const DESTRUCTION_SIMULATION_PROFILE& profile,
		bool_t preserveSampleTime,
		bool_t playAfterStage);
	bool_t Stage_DestructionElementDraftPreview();
	void Select_DestructionSimulationProfile(const std::string& profileId);
	void Select_DestructionSimulationElement(const std::string& elementId);
	void Select_DestructionSimulationFragment(
		const std::string& elementId,
		const std::string& fragmentId);
	void Reset_DestructionSimulationUI();
	const DESTRUCTION_SIMULATION_PROFILE*
		Get_SelectedDestructionSimulationProfile() const;
	bool_t Refresh_DestructionHighlight();
	void Apply_DestructionPreview(DEPLOY_PROP_STATE state);
	void Restore_DestructionPreview();

	/* Queries */
	PLACED_ENTRY* Find_Placement(uint64_t placementId);
	const MAP_ASSET_ENTRY* Get_SelectedAsset() const;

private:
	/* Shared Tool State */
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	bool_t m_bOpen = false;
	uint32_t m_iAuthoringLevelIndex = ETOUI(LEVEL::END);
	TOOL_MODE m_eToolMode = TOOL_MODE::MAP_ASSETS;
	std::vector<EDITOR_AREA_DESCRIPTOR> m_EditorAreas;
	size_t m_iActiveEditorArea = SIZE_MAX;
	size_t m_iPendingEditorArea = SIZE_MAX;
	bool_t m_isEditorAreaSwitchPending = false;
	bool_t m_isEditorExitPending = false;
	std::unordered_map<std::wstring, std::filesystem::path>
		m_PrototypeModelPaths;

	/* World Interaction State */
	bool_t m_bPreviousMouseDown = false;

	/* Map Asset State */
	bool_t m_bDirty = false;
	PLACEMENT_STATE m_ePlacementState = PLACEMENT_STATE::IDLE;

	CMapAssetCatalog m_Catalog;
	CDeployPropRuntime m_DeployRuntime;
	std::unique_ptr<CMapAssetPreview> m_pAssetPreview;
	std::string m_SelectedAssetId;
	std::string m_Status = "Enter AssetTest with F2";
	char m_Filter[128]{};
	std::unordered_set<std::string> m_FavoriteAssetIds;

	vector<PLACED_ENTRY> m_Placements;
	vector<STATIC_BATCH_ENTRY> m_StaticBatches;
	DEPLOY_PROP_STATE m_DeployPhase = DEPLOY_PROP_STATE::INTACT;
	ENVIRONMENT_PHASE m_EnvironmentPhase = ENVIRONMENT_PHASE::BASELINE;
	bool_t m_bShowBernLandscape = false;
	uint64_t m_iSelectedPlacementId = {};
	uint64_t m_iNextPlacementId = 1;

	/* World Gameplay State */
	CWorldGameplayDocument m_WorldGameplayDocument;
	vector<TRIGGER_BOX_ENTRY> m_WorldTriggerBoxes;
	vector<NPC_PREVIEW_ENTRY> m_WorldNpcPreviews;
	int32_t m_iWorldNpcArchetypeIndex = 0;
	CSpawnGroupDocument m_SpawnGroupDocument;
	vector<TRIGGER_BOX_ENTRY> m_SpawnAnchorBoxes;
	bool_t m_bWorldGameplayDirty = false;
	bool_t m_bSpawnGroupsDirty = false;
	bool_t m_bWorldGameplayPlacementArmed = false;
	bool_t m_bWorldTriggerTargetPickArmed = false;
	bool_t m_bSpawnAnchorPlacementArmed = false;
	WORLD_PLACEMENT_KIND m_eWorldPlacementKind =
		WORLD_PLACEMENT_KIND::PLAYER_SPAWN;
	std::string m_SelectedWorldPlacementId;
	std::string m_WorldGameplayStatus =
		"Open Map Tool from F1 Developer Tools";
	char m_WorldPlacementId[128] = "player.spawn.editor";
	char m_WorldArchetypeId[128] = "";
	char m_WorldEncounterId[128] = "";
	char m_SpawnAnchorId[128] = "anchor.valtan.editor";
	char m_SpawnGroupId[128] = "spawn.valtan.stage01";
	char m_SpawnWaveId[128] = "wave.01";
	std::string m_SelectedSpawnAnchorId;
	std::string m_SelectedSpawnGroupId;
	std::string m_SelectedSpawnWaveId;
	uint32_t m_iSpawnArchetypeOption = 0;
	uint32_t m_iSpawnEntryCount = 1;
	uint32_t m_iSpawnInitialDelayMs = 0;
	uint32_t m_iSpawnIntervalMs = 250;
	uint32_t m_iSpawnWaveStartDelayMs = 0;
	uint32_t m_iSpawnGroupMaxAlive = 8;
	float3_t m_WorldPlacementPositionDelta = {};
	float3_t m_WorldTriggerHalfExtents = float3_t(2.f, 1.f, 2.f);
	bool_t m_bWorldTriggerOnce = true;

	/* World Destruction State */
	CEncounterPatternReference m_EncounterReference;
	std::string m_EncounterReferenceStatus =
		"Press Reload Encounter Reference";
	std::string m_SelectedDestructionPatternId;
	uint64_t m_iSelectedDeployPlacementId = 0;
	bool_t m_bDestructionOnlyWithOffAction = false;
	char m_DestructionDeployFilter[128]{};
	CWorldDestructionDocument m_DestructionDocument;
	std::filesystem::path m_WorldEventsPath;
	std::string m_DestructionStatus =
		"Select Valtan to author world destruction";
	std::string m_SelectedDestructionGroupId;
	std::string m_SelectedDestructionBindingId;
	/* Which encounter stage the next binding will attach to. Set from the
	   timeline; not a binding identity. */
	std::string m_SelectedDestructionStageId;
	vector<TRIGGER_BOX_ENTRY> m_DestructionHighlightBoxes;
	vector<std::pair<uint64_t, DEPLOY_PROP_STATE>>
		m_DestructionPreviewPreviousStates;
	bool_t m_bDestructionPickArmed = false;
	bool_t m_bDestructionAddMemberArmed = false;
	bool_t m_bDestructionAdvancedMode = false;
	bool_t m_bDestructionOnlyUnassigned = false;
	bool_t m_bDestructionBindingEnabled = false;
	bool_t m_bDestructionNewSettingArmed = false;
	bool_t m_bDestructionTimelinePlaying = false;
	bool_t m_bDestructionTimelineLoop = true;
	f32_t m_fDestructionTimelineMs = 0.f;
	char m_DestructionGroupId[129] = "destroyable.group.valtan.wall.01";
	char m_DestructionMutationId[129] = "mutation.valtan.wall.01.break";
	char m_DestructionBindingId[129] = "binding.valtan.wall.01.break";
	char m_DestructionReceiverId[129] = "collision.valtan.wall.01";
	char m_DestructionRegionId[129]{};
	int32_t m_iDestructionTriggerKind = 1;
	int32_t m_iDestructionBreakingMs = 1900;
	int32_t m_iDestructionOffsetMs = 0;

	/* Destruction Physics Audition State */
	std::unique_ptr<CDestructionSimulationController>
		m_pDestructionSimulationController;
	CDestructionSimulationDocument m_DestructionSimulationDocument;
	std::string m_SelectedDestructionSimulationProfileId;
	std::string m_SelectedDestructionSimulationElementId;
	std::string m_SelectedDestructionSimulationFragmentId;
	std::optional<DESTRUCTION_SIMULATION_ELEMENT>
		m_DestructionSimulationElementDraft;
	std::string m_DestructionSimulationStatus =
		"Select a destruction group and stage a simulation";
	char m_DestructionSimulationFilter[128]{};
	char m_DestructionSimulationReceiverId[128]{};
	bool_t m_bDestructionSimulationElementDraftDirty = false;
	bool_t m_bDestructionDebrisPrototypesReady = false;
	std::string m_DestructionDebrisPrototypeStatus =
		"PROJECT_AUTHORED debris models are not admitted";
	bool_t m_bDestructionSimulationLoop = true;
	bool_t m_bDestructionSimulationClearRequested = false;

	/* Navigation State */
	bool_t m_bNavigationStrokeActive = false;
	NAVIGATION_MODE m_eNavigationMode =
		NAVIGATION_MODE::BAKE;
	NAV_BOUNDS_STATE m_eNavigationBoundsState =
		NAV_BOUNDS_STATE::IDLE;
	NAVGRID_BAKE_DESC m_NavigationBakeDesc;
	std::string m_NavigationBakeStatus = "Create Nav Bounds";
	bool_t m_bNavigationBakeResetConfirmed = false;
	bool_t m_bNavigationBakeResetPending = false;

	NAVIGATION_EDIT_ACTION m_eNavigationEditAction =
		NAVIGATION_EDIT_ACTION::APPLY;
	uint32_t m_iBrushRadius = {};
	/* Cells without a baked surface carry no height, so the overlay has to
	   draw them on the Nav Bounds floor. On a large bake they outnumber the
	   real surface cells and hide it, so they stay off unless asked for. */
	bool_t m_bShowUnresolvedCells = false;

	CNavGridPaintDocument m_NavigationDocument;
	CNavRuntimeBlockerDocument m_RuntimeBlockerDocument;
	std::filesystem::path m_NavigationSourcePath;
	std::filesystem::path m_NavigationPaintPath;
	std::filesystem::path m_NavigationRuntimePath;
	std::filesystem::path m_RuntimeBlockerPath;
	std::string m_NavigationStatus = "Open ASSET_TEST with F2";
	size_t m_iSelectedRuntimeRegion = {};
	char m_RuntimeBlockerId[128] =
		"VALTAN_OUTER_RING_COLLAPSE";
	char m_RuntimeConditionId[128] =
		"VALTAN_ARENA_DESTROYED";
	bool_t m_RuntimeActivateWhenConditionTrue = true;
	std::unordered_map<std::string, bool_t> m_NavigationConditions;
	std::unique_ptr<NAVIGATION_RENDER_RESOURCES>
		m_pNavigationRenderResources;

	/* Camera State */
	weak_ptr<CCamera_Free> m_pAssetTestCamera;
	std::string m_CameraStatus = "Open ASSET_TEST with F2";
};

NS_END
