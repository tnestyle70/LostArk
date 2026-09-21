#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapPlacementDocument.h"
#include "MapPlacementRuntime.h"
#include "WorldSequencePlayer.h"
#include "MapLightPresentationRuntime.h"
#include "DeployPropRuntime.h"
#include "DestructionSimulationDocument.h"
#include "EncounterPatternReference.h"
#include "WorldDestructionDocument.h"
#include "NavGridBaker.h"
#include "NavGridPaintDocument.h"
#include "MapNavigationContract.h"
#include "NavRuntimeBlockerDocument.h"
#include "WorldGameplayDocument.h"
#include "SpawnGroupDocument.h"

/* The integrated cutscene view borrows the Composition session the Sequencer
   shell already owns. Declared, not included, so this header stays light. */
NS_BEGIN(Client)
class ICompositionWorkbenchSession;
NS_END

#include <memory>
#include <utility>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;
class CMapAssetPreview;
class CMapStaticBatchObject;
class CTrigger_Box;
class CNpc;
class CCamera_Free;
class CDestructionSimulationController;
class CWorldSequenceToolPanel;
struct VALTAN_CINEMATIC_CAMERA_POSE;
class CMapTool final
{
private:
	enum class TOOL_MODE
	{
		MAP_ASSETS,
		WORLD_GAMEPLAY,
		WORLD_DESTRUCTION,
		WORLD_SEQUENCE,
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

	/* One authored camera shot. The product level holds this exact pose while
	   the local Character stands inside the box, so what is framed here is
	   what ships. */
	struct EDITOR_CAMERA_KEYFRAME final
	{
		/* Stable within its shot; the runtime rejects duplicates. Never a
		   list index, so reordering keys cannot silently rebind one. */
		std::string sceneId;
		int32_t timeMs = 0;
		float3_t eye = {};
		float3_t lookAt = {};
		f32_t fovYDegrees = 50.f;
		float3_t up = { 0.f, 1.f, 0.f };
		bool_t hasUp = false;
	};

	struct EDITOR_CAMERA_SHOT final
	{
		std::string shotId;
		std::string displayName;
		int32_t defaultHoldMs = 3000;
		bool_t patternOnly = false;
		bool_t linearTransition = false;
		/* Empty means the box decides. Naming a sequence instance makes
		   the shot hold for exactly that sequence. */
		std::string sequenceInstanceId;
		float3_t center = {};
		float3_t halfExtents = float3_t(8.f, 4.f, 8.f);
		f32_t yawDegrees = 0.f;
		float3_t eye = {};
		float3_t lookAt = {};
		f32_t fovYDegrees = 60.f;
		int32_t blendInMs = 2000;
		int32_t blendOutMs = 1000;
		int32_t priority = 10;
		/* On: the shot slides with the local Character instead of pinning
		   one pose. Both offsets are added to that Character's position. */
		bool_t followsPlayer = false;
		float3_t followEyeOffset = {};
		float3_t followLookAtOffset = {};
		/* An empty list keeps the shot pose; one key holds an explicit pose
		   and two or more keys animate on the product sequence clock. */
		std::vector<EDITOR_CAMERA_KEYFRAME> keyframes;
		int32_t trackDurationMs = 0;
		int32_t interpolationIndex = 1;
		int32_t easingIndex = 1;
	};

	/* One camera occurrence inside a cutscene. The shot owns the framing and
	   its own key times; this owns only where that shot starts on the whole
	   cutscene clock, so two cutscenes can reuse one shot at different times. */
	struct EDITOR_CUTSCENE_CUT final
	{
		std::string cutId;
		std::string shotId;
		int32_t startMs = 0;
	};

	/* The authoring-side cutscene: the list the editor picks from. It is not a
	   combat pattern and never auto-plays; it exists so one Play shows every
	   camera cut and every World actor on one clock. */
	struct EDITOR_CUTSCENE final
	{
		std::string cutsceneId;
		std::string displayName;
		/* The whole cutscene, which outlives the last camera cut when the
		   original hands the camera back before the actors finish. */
		int32_t durationMs = 0;
		std::vector<EDITOR_CUTSCENE_CUT> cameraCuts;
		/* World Sequence instances this cutscene drives. Empty is valid and
		   means camera-only, which must still play. */
		std::vector<std::string> worldInstanceIds;
	};

	enum class EDITOR_CUTSCENE_STATE
	{
		STOPPED,
		PLAYING,
		PAUSED,
	};

	struct EDITOR_AREA_DESCRIPTOR
	{
		std::string areaId;
		std::string label;
		std::filesystem::path sourceCatalog;
		std::filesystem::path sourcePlacements;
		std::filesystem::path sourceMaterials;
		std::filesystem::path sourceLights;
		std::filesystem::path sourceDeployCatalog;
		std::filesystem::path sourceDeployPlacements;
		std::filesystem::path navigationSource;
		std::filesystem::path navigationPaint;
		std::filesystem::path navigationBlockers;
		std::filesystem::path gameplayDocument;
		std::filesystem::path encounterReference;
		std::filesystem::path worldEventsDocument;
		std::filesystem::path destructionSimulationDocument;
		std::filesystem::path cameraShotDocument;
		EDITOR_NAVIGATION_POLICY navigationPolicy =
			EDITOR_NAVIGATION_POLICY::NONE;
		EDITOR_GAMEPLAY_POLICY gameplayPolicy =
			EDITOR_GAMEPLAY_POLICY::NONE;
		bool_t allowNavigationBootstrap = false;
	};

	/* An Area switch admits every model of the target Area before it stages a
	   single placement. Bern alone is 1003 prototypes over 1.8 GB, so that
	   admission runs on a per-frame budget and the editor keeps drawing its
	   progress instead of stalling inside one frame. */
	struct EDITOR_AREA_PRELOAD_STATE final
	{
		size_t iDescriptorIndex = SIZE_MAX;
		CMapAssetCatalog Catalog;
		size_t iNextEntry = 0;

		bool_t Is_Active() const noexcept
		{
			return SIZE_MAX != iDescriptorIndex;
		}
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
	static bool_t Save_CameraShotDocumentAtomic(const std::filesystem::path& path,
		std::string_view expectedText, std::string_view text, std::string& outStatus);
	CMapTool();
	~CMapTool();

	HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	void Toggle();
	void SetOpen(bool_t isOpen);
	/* Multiple authoring windows may stay visible at once. Only the workspace's
	   focused input owner may mutate the world viewport; background document,
	   preload and playback updates continue for every open Map Tool window. */
	void Update(f32_t fTimeDelta, bool_t bAllowWorldInput = true);
	void Render();

	bool IsOpen() const;
    void Collect_WorldSequenceSubtitles(std::vector<WORLD_SEQUENCE_SUBTITLE_SAMPLE>& out) const
    { m_ArenaRisePlayer.Collect_Subtitles(out); }

	bool_t ConsumesWorldLeftMouse() const;

private:
	/* Frame Update */
	bool_t Is_MapAuthoringLevel() const;
	CWorldSequencePlayer::TARGET_SET Runtime_AuthoringTargets() const;
	vector<PLACED_ENTRY>& Authoring_Placements();
	const vector<PLACED_ENTRY>& Authoring_Placements() const;
	vector<STATIC_BATCH_ENTRY>& Authoring_Batches();
	const vector<STATIC_BATCH_ENTRY>& Authoring_Batches() const;
	CDeployPropRuntime& Authoring_Deploy();
	const CDeployPropRuntime& Authoring_Deploy() const;
	bool_t Can_ChangeRuntimeStructure();
	/* The runtime map belongs to whichever arena Level is current. */
	void Apply_RuntimeAuthoringActive(bool_t active);
	bool_t Can_ReplaceRuntimeAuthoringTargets() const;
	void Remember_RuntimePlacement(const MAP_PLACEMENT_RECORD& record);
	void Reset_RuntimePlacementDraft(const vector<MAP_PLACEMENT_RECORD>& records);
	void Forget_RuntimePlacement(uint64_t placementId);
	void Rebase_RuntimeMotions();
	const MAP_PLACEMENT_RECORD& Authored_Placement(const PLACED_ENTRY& entry) const;
	void Update_DestructionSimulation(
		f32_t fTimeDelta,
		bool_t isMapAuthoringLevel);
	void Update_WorldInteraction(bool_t isAssetTest);
	void Handle_LevelTransition(
		uint32_t currentLevelIndex,
		bool_t isMapAuthoringLevel);

	/* Frame Render */
	void Render_WorldOverlay(bool_t isAssetTest);
	void Render_WorldNpcRouteOverlay();
	void Render_WorldNpcBatchOverlay();
	void Render_WorkspaceBar(bool_t isAssetTest);
	void Render_ActiveMode(bool_t isAssetTest);
	void Render_MapAssetsPanel(bool_t isAssetTest);
	void Render_WorldGameplayPanel(bool_t isAssetTest);
	void Render_SpawnGroupsPanel();
	void Render_WorldDestructionPanel(bool_t isAssetTest);
	void Render_WorldSequencePanel(bool_t isAssetTest);
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
	void Render_AnimatedPropsAuthoring();
	/* Authoring keeps the arena visible so it can be edited. This previews the
	   product level's cutscene start state without touching a saved document. */
	void Render_CutsceneArenaPreview();
	/* Runs the four card maze march states on the editor Level so the
	   crossing can be watched without a Server or an arena entry. */
	bool_t Play_CardMiroMarch();
	void Stop_CardMiroMarch();
	void Apply_CutsceneCameraTrack(f32_t timeDelta);
	void End_CutsceneCameraTrack();
	/* Marshals a shot's authored track into the one cinematic cue the
	   product samples, then samples it on the given clock. A shot with
	   fewer than two keys yields its single pose. */
	bool_t Sample_ShotCameraTrack(
		const EDITOR_CAMERA_SHOT& shot,
		f32_t elapsedMs,
		VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	/* Plays one stage's intro camera shot -- the shot bound to
	   `world.sequence.instance.mario_<stage>_intro` -- from the top, with
	   no player and no Server, and hands the camera back when it ends. */
	vector<std::string> Collect_MarioIntroStages() const;
	bool_t Play_MarioIntro(const std::string& stageToken);
	void Update_MarioIntro();
	void Stop_MarioIntro();
	void Apply_CutsceneArenaVisibility(bool_t hidden);
	bool_t Is_CutsceneOriginalPlaying() const;
	void Hide_CutsceneSet();
	void Release_CutsceneBookPreview(uint64_t placementId);
	void Update_CutsceneArenaRise(
		f32_t fTimeDelta, bool_t isMapAuthoringLevel);
	/* The generated rise is split across instances by the 32-track template
	   limit, so previewing the whole arena starts all of them together. */
	bool_t Play_CutsceneArenaRise();
	bool_t Play_CutsceneOriginalRise();
	/* The camera track runs on the clock of the sequence its own shot
	   names, so key editing asks about that sequence rather than about
	   the one cutscene this Area happens to ship with. */
	bool_t Is_ShotCutsceneClockPlaying(
		const EDITOR_CAMERA_SHOT& shot) const;
	bool_t Ensure_ShotCutsceneClock(const EDITOR_CAMERA_SHOT& shot);
	/* Editor cutscene session. The clock lives here rather than in the World
	   player so a cutscene with no World actors still plays: the camera is
	   sampled from absolute session time, and World instances, when the
	   cutscene names any, are seeked to that same time. */
	/* The full target set for the Area being edited, including the device and
	   context the object-resource actors need to build their models. */
	bool_t Build_CutsceneTargets(CWorldSequencePlayer::TARGET_SET& outTargets);
	/* Only the Kouku arena Level adds the World Object prototype under its own
	   index. The isolated editor shell and every other runtime Level need the
	   tool to register it, or each actor clone fails and the cutscene plays
	   over an empty stage. */
	bool_t Ensure_WorldObjectPrototype();
	const EDITOR_CUTSCENE* Find_EditorCutscene(
		const std::string& cutsceneId) const;
	const EDITOR_CAMERA_SHOT* Find_CameraShot(
		const std::string& shotId) const;
	/* The cut that owns T, plus the local time inside its shot. Null between
	   cuts, which is an authored gap and hands the camera back. */
	const EDITOR_CUTSCENE_CUT* Find_CutsceneCutAt(
		const EDITOR_CUTSCENE& cutscene,
		f32_t timeMs,
		f32_t& outLocalMs) const;
	bool_t Play_EditorCutscene(const std::string& cutsceneId);
	void Stop_EditorCutscene();
	void Update_EditorCutscene(f32_t fTimeDelta);
	bool_t Apply_EditorCutsceneCamera();
	bool_t Prepare_EditorCutsceneWorld(const EDITOR_CUTSCENE& cutscene);
	void Seek_EditorCutsceneWorld();
	void Render_CutsceneSection();
	/* Stops every World instance this session started, whether or not it is
	   still healthy, so a partial failure never leaves survivors behind. */
	void Release_EditorCutsceneWorld(bool_t restorePlacements);
	/* A Level transition has already torn the old Level down: only the tool's
	   own references are dropped, with no placement restore. */
	void Abandon_EditorCutscene(const std::string& reason);
	/* Re-admits the World draft and replays the session actors after an edit,
	   so the next frame shows the unsaved change at the same time. */
	bool_t Refresh_EditorCutsceneWorldDraft();
	void Render_CutsceneActorSection(const EDITOR_CUTSCENE& cutscene,
		bool_t isSession);
	/* Saves this Area's camera shots and World sequences as one unit: both
	   drafts are validated first, and a failed second write restores the
	   first. Nothing is published. */
	bool_t Save_CutsceneAuthoring();
	/* Replays every authored Mario sequence for as long as the editor asks,
	   so a trigger box can be placed against motion that is on screen
	   instead of against a coordinate. */
	bool_t Toggle_MarioSequenceLoop();
	void Update_MarioSequenceLoop(
		f32_t fTimeDelta,
		const CWorldSequencePlayer::TARGET_SET& targets);
	void Render_Palette(f32_t childHeight);
	void Render_Hierarchy(f32_t childHeight);
	void Render_Inspector();
	void Render_AssetPreview();
	void Render_DecoderReport() const;

	/* Camera Runtime */
	bool_t Find_AssetTestCamera();
	bool_t Focus_ActiveEditorAreaCamera();
	void Rebuild_EditorSublevelJumps();
	bool_t Jump_ToEditorSublevel(size_t jumpIndex);
	void Update_EditorSublevelJumpShortcuts();

	/* Navigation Document and Runtime */
	bool_t Load_NavigationDocument();
	bool_t Load_RuntimeBlockers();
	bool_t Register_RuntimeBlockers();
	bool_t Set_NavigationCondition(
		const std::string& conditionId,
		bool_t value);
	bool_t Save_Navigation();
	/* Resolves the paths of the grid currently selected in the Navigation
	   panel: the Area's base grid when no region is selected, otherwise
	   "<AreaId>.<regionId>". */
	bool_t Resolve_SelectedNavigationContract(
		MAP_NAVIGATION_CONTRACT& outContract,
		std::string& outStatus) const;
	/* Switches the panel to another grid and reloads its documents. A failed
	   load restores the previous selection so the editor never shows one
	   grid's paint over another grid's cells. */
	bool_t Select_NavigationRegion(std::string regionId);
	/* Adds the freshly baked region to the Area manifest. Called only after
	   Bake_Navigation has written its navsource. */
	bool_t Commit_NavigationRegionManifest();
	void Render_NavigationRegionControls();
	bool_t Load_CameraShots(const EDITOR_AREA_DESCRIPTOR& descriptor);
	bool_t Save_CameraShots();
	/* Writer text for the current draft. The revision is a parameter so a
	   dirty check can compare drafts without reading the file. */
	std::string Build_CameraShotDocumentText(const std::string& areaId,
		uint32_t revision) const;
	bool_t Is_CameraShotDraftDirty() const;
	/* Shot IDs unique and non-empty, box extents positive: what the writer
	   requires before any byte of a camera document is written. */
	bool_t Validate_CameraShotDraft(std::string& outError) const;
	/* Parses one camera document without touching the loaded state, so a bad
	   Reload keeps the draft, the selection and the reason on screen. */
	static bool_t Parse_CameraShotDocument(const std::string& text,
		const std::string& areaId, std::vector<EDITOR_CAMERA_SHOT>& outShots,
		std::vector<EDITOR_CUTSCENE>& outCutscenes, std::string& outError);
	void Render_IntegratedCutsceneView();
	void Render_CameraTrackTimeline(EDITOR_CAMERA_SHOT& shot);
	void Render_CameraShotSection();
	void End_CameraShotPreview();
	/* Preview-only Valtan Arena outer-wall overlay. It addresses the exact
	   destruction-owned Deploy ring and remains independent from Server state,
	   cutscene sampling, floor, water and all ordinary map placements. */
	bool_t Set_CameraPreviewSurroundingsCleared(bool_t cleared);
	bool_t Refresh_CameraPreviewSurroundings();
	/* Picks rendered surface height, including cells with no baked floor.
	   Failed picks explain their cause through m_NavigationStatus. */
	bool_t Try_PickNavigationCell(
		int32_t& outCellX,
		int32_t& outCellZ,
		f32_t& outWorldY);
	bool_t Try_PaintNavigation();
	bool_t Try_PlaceNavigationBounds();
	/* Bake is split so pressing it is not already the irreversible act.
	   Preview builds the grid in memory and reports what it found; only
	   Bake_Navigation writes navsource and drops the incompatible paint. */
	bool_t Preview_NavigationBake();
	void Discard_NavigationBakePreview();
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
	bool_t Admit_AuthoringPrototype(const MAP_ASSET_ENTRY& asset);
	bool_t Ensure_DeployAuthoringPrototypes(
		const CDeployPropCatalog& catalog);
	/* The arena attach is decided after this runs, so it is passed in. */
	bool_t Ensure_DestructionDebrisAuthoringPrototypes(bool_t runtimeAttach);
	bool_t Load_EditorAreaRegistry();
public:
	/* MainApp hands over the same session pointer the Sequencer shell uses.
	   One owner, one draft: this tool never copies the document. */
	void Set_SequenceCompositionSession(ICompositionWorkbenchSession* session)
	{ m_pSequenceCompositionSession = session; }
	/* Valtan edits its own split source, so the integrated view hosts the
	   Valtan workbench in that Area instead of the KoukuSaydon Sequence
	   session, which only reads the KoukuSaydon composition. */
	void Set_ValtanCompositionSession(ICompositionWorkbenchSession* session)
	{ m_pValtanCompositionSession = session; }
	/* The Object session owns Motion authoring. The integrated view shows its
	   detail beneath the timeline so a Motion edit stays on one screen. */
	void Set_ObjectCompositionSession(ICompositionWorkbenchSession* session)
	{ m_pObjectCompositionSession = session; }
	ICompositionWorkbenchSession* Get_HostedObjectSession() const
	{ return m_bHostingObjectSession ? m_pObjectCompositionSession : nullptr; }
	/* Stable across the frame order, unlike the per-frame host flag: the
	   preview route runs before this tool renders. */
	bool_t Is_IntegratedCutsceneViewOpen() const
	{ return m_bIntegratedCutsceneView; }
	/* The concrete owners live in MainApp, so this tool only raises the ask and
	   displays what MainApp reports back. */
	void Set_IntegratedSaveState(bool_t sequenceDirty, bool_t objectDirty,
		std::string status)
	{
		m_bIntegratedSequenceDirty = sequenceDirty;
		m_bIntegratedObjectDirty = objectDirty;
		if (!status.empty()) m_IntegratedSaveStatus = std::move(status);
	}
	bool_t Consume_IntegratedSaveRequest()
	{ return std::exchange(m_bIntegratedSaveRequested, false); }
	/* True while this tool owns the session frame for the current frame, so
	   the Sequencer shell skips its own Begin/End for that session. */
	bool_t Is_HostingCompositionSession() const
	{ return m_bHostingCompositionSession; }
	/* Which session the integrated view actually opened this frame. The shell
	   suppresses that exact session, so an Area switch cannot leave the other
	   one drawn twice. */
	ICompositionWorkbenchSession* Get_HostedCompositionSession() const
	{ return m_bHostingCompositionSession ? m_pHostedCompositionSession : nullptr; }
	std::string Debug_GetActiveAreaId() const;
	shared_ptr<CCamera_Free> Debug_GetCamera() const { return m_pAssetTestCamera.lock(); }
	int Debug_WorldLevelSelection(const std::string& areaId, uint64_t placementId,
		bool_t deploy, std::string& status);
	// -1 rejected, 0 Area preparation in progress, 1 completed. No authoring writes.
	int Debug_SequenceViewer(const std::string& areaId, const std::string& sequenceId,
		const std::string& triggerId, bool_t play, bool_t stop, const float3_t* focus, std::string& status);
private:
	int Debug_PrepareEditorArea(const std::string& areaId, std::string& status);
	bool_t Begin_EditorAreaSwitch(size_t descriptorIndex);
	void Update_EditorAreaPreload();
	void Report_EditorAreaPreloadProgress();
	bool_t Switch_EditorArea(size_t descriptorIndex);
	bool_t Save_AllAuthoring();
	bool_t Save_PlacementsAndWorldSequences();
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
	bool_t Save_Placements(
		bool_t linkedTransactionAlreadyLocked = false,
		vector<MAP_PLACEMENT_RECORD>* outSavedRecords = nullptr);
	bool_t Load_Placements();
	bool_t Load_DeployProps();
	bool_t Stage_DeployProps(
		const EDITOR_AREA_DESCRIPTOR& descriptor,
		CDeployPropRuntime& outRuntime);
	bool_t Commit_DeployCatalog(
		CDeployPropCatalog catalog,
		const std::string& successStatus);
	bool_t Save_DeployPlacements();
	bool_t Try_PlaceSelectedDeploy();
	bool_t Apply_AnimatedPropTransform();
	bool_t Remove_SelectedAnimatedProp();
	uint64_t Allocate_AnimatedPropPlacementId() const;
	const DEPLOY_PROP_ASSET_ENTRY* Get_SelectedDeployAsset() const;
	const DEPLOY_RUNTIME_ENTRY* Get_SelectedAnimatedProp() const;
	void Sync_AnimatedPropTransformDraft();
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
	std::string Allocate_WorldNpcPlacementId(
		const std::string& archetypeId) const;
	bool_t Try_PickWorldTriggerTarget();
	bool_t Try_PickWorldNpcWaypoint();
	bool_t Try_PickWorldNpcBatchCenter();
	bool_t Place_WorldNpcBatch();
	bool_t Commit_WorldNpcBatch();
	void Sync_WorldNpcBehaviorDraft(
		const WORLD_GAMEPLAY_PLACEMENT& placement);
	bool_t Apply_WorldNpcBehaviorDraft();
	bool_t Validate_WorldNpcBehaviorNavigation(
		const WORLD_GAMEPLAY_PLACEMENT& placement,
		const CNavGridPaintDocument& navigation,
		const CNavRuntimeBlockerDocument& blockers,
		std::string& outStatus) const;
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
		vector<NPC_PREVIEW_ENTRY>& outEntries,
		const CNavGridPaintDocument* pNavigation = nullptr,
		const CNavRuntimeBlockerDocument* pBlockers = nullptr);
	void Remove_WorldNpcPreviews(vector<NPC_PREVIEW_ENTRY>& entries);
	void Update_WorldTriggerBoxPresentation(bool_t isVisible);
	std::filesystem::path Get_WorldGameplayPath() const;
	std::filesystem::path Get_SpawnGroupsPath() const;

	/* Reusable World Sequence Authoring */
	std::filesystem::path Get_WorldSequencePath() const;

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
	EDITOR_AREA_PRELOAD_STATE m_EditorAreaPreload;
	std::unordered_map<std::wstring, std::filesystem::path>
		m_PrototypeModelPaths;

	/* World Interaction State */
	bool_t m_bPreviousMouseDown = false;

	/* Map Asset State */
	bool_t m_bDirty = false;
	PLACEMENT_STATE m_ePlacementState = PLACEMENT_STATE::IDLE;

	CMapAssetCatalog m_Catalog;
	// The arena owns live objects. This full-source draft excludes sampled animation poses.
	bool_t m_bRuntimeAuthoring = false;
	vector<MAP_PLACEMENT_RECORD> m_RuntimePlacementDraft;
	std::unordered_map<uint64_t, size_t> m_RuntimePlacementIndex;
	CDeployPropRuntime m_DeployRuntime;
	bool_t m_bDeployDirty = false;
	bool_t m_bAnimatedPropPlacementArmed = false;
	std::string m_SelectedDeployAssetId;
	char m_AnimatedPropFilter[128]{};
	uint64_t m_iSelectedAnimatedPropPlacementId = 0;
	uint64_t m_iAnimatedPropDraftPlacementId = 0;
	float3_t m_AnimatedPropDraftPosition = {};
	float4_t m_AnimatedPropDraftRotation = float4_t(0.f, 0.f, 0.f, 1.f);
	f32_t m_fAnimatedPropDraftScale = 1.f;
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	std::unique_ptr<CMapAssetPreview> m_pAssetPreview;
	std::string m_SelectedAssetId;
	std::string m_Status = "Enter AssetTest with F2";
	std::string m_CompletePlayStatus =
		"Complete Play uses the workspace's selected saved Server pattern.";
	char m_Filter[128]{};
	std::unordered_set<std::string> m_FavoriteAssetIds;

	vector<PLACED_ENTRY> m_Placements;
	/* Idle EFActorMotion for the loaded area, bound to the entries above.
	   Rebound when the active area changes so a stale index never writes
	   onto a different placement. */
	vector<MAP_RUNTIME_SELF_MOTION_ENTRY> m_SelfMotions;
	std::unordered_map<std::string, shared_ptr<Engine::CModel>> m_SelfMotionModels;
	std::string m_strSelfMotionAreaId;
	f32_t m_fSelfMotionElapsedSeconds = 0.f;
	vector<STATIC_BATCH_ENTRY> m_StaticBatches;
	DEPLOY_PROP_STATE m_DeployPhase = DEPLOY_PROP_STATE::INTACT;
	ENVIRONMENT_PHASE m_EnvironmentPhase = ENVIRONMENT_PHASE::BASELINE;
	bool_t m_bShowBernLandscape = false;
	/* Preview-only. Never saved, so reopening the Area restores full visibility. */
	bool_t m_bCutsceneArenaHidden = false;
	/* Visibility each arena placement had before the preview hid it. Culling
	   boxes are invisible helpers, so a blanket restore would reveal them. */
	vector<std::pair<uint64_t, bool_t>> m_CutsceneArenaRestoreVisibility;
	/* Same playback path the product level uses, so the preview cannot drift
	   from what the cutscene will actually do. */
	CWorldSequencePlayer m_ArenaRisePlayer;
	bool_t m_bArenaRiseAreaLoaded = false;
	/* Which Area's World document the player currently holds. Kouku was the
	   only caller once, so the id was implicit; an Area cutscene has to know
	   when the loaded document belongs to a different map. */
	std::string m_strArenaRiseLoadedArea;
	/* The Mario instances resolved once when the loop starts, so the
	   document is not walked every frame. Empty means the loop is off. */
	vector<std::string> m_MarioLoopInstanceIds;
	bool_t m_bMarioSequenceLoopRunning = false;
	/* Negative means no cutscene is running. Stays at zero while the arena
	   instances play and counts up once they have settled, so the book can be
	   despawned after a short hold. */
	f32_t m_fCutsceneBookHoldMs = -1.f;
	/* True while the original cutscene owes the arena back. */
	bool_t m_bCutsceneOriginalRunning = false;
	/* True while the cutscene preview holds the editor camera. */
	bool_t m_bCutsceneCameraHeld = false;
	/* Authoring scrub: the cutscene holds on one frame so a key can be moved
	   and judged in place. Negative means the preview runs normally. */
	f32_t m_fCutsceneScrubMs = -1.f;
	int32_t m_iCutsceneSelectedKey = -1;
	/* Looping one key's own stretch. Negative start means no loop is armed. */
	f32_t m_fCutsceneLoopStartMs = -1.f;
	f32_t m_fCutsceneLoopEndMs = -1.f;
	/* The pose the preview held when the shot took over, frozen once so
	   advancing both ends of the blend cannot shorten it. */
	float3_t m_vCutsceneCameraFromEye = {};
	float3_t m_vCutsceneCameraFromLook = {};
	f32_t m_fCutsceneCameraFromFov = 60.f;
	f32_t m_fCutsceneCameraBlendSeconds = 0.f;
	/* Stage intro preview. The flag keeps the camera track gate open for
	   as long as the bound sequence plays. */
	bool_t m_bMarioIntroRunning = false;
	std::string m_MarioIntroInstanceId;
	std::string m_MarioWalkStatus =
		"No stage intro has run in this session";
	std::string m_CardMiroMarchStatus =
		"No card maze march has run in this session";
	/* The arena Level registers the world object prototype; the editor
	   Level does not, so the first march here registers it. */
	bool_t m_bWorldObjectPrototypeReady = false;
	/* Why the prototype is ready or not: registered here, already present on
	   the Level, owned by the Kouku Level, or a real creation failure. */
	std::string m_strWorldObjectPrototypeStatus;
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
	bool_t m_bWorldNpcWaypointPickArmed = false;
	bool_t m_bWorldNpcBatchCenterPickArmed = false;
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
	std::string m_WorldNpcBehaviorDraftPlacementId;
	std::optional<WORLD_NPC_BEHAVIOR> m_WorldNpcBehaviorDraft;
	bool_t m_bWorldNpcBehaviorDraftDirty = false;
	bool_t m_bWorldNpcContinuousPlacement = false;
	bool_t m_bWorldNpcBrushRandomYaw = true;
	std::optional<WORLD_GAMEPLAY_PLACEMENT> m_WorldNpcBrushPreset;
	f32_t m_fWorldNpcQuickWanderRadius = 5.f;
	f32_t m_fWorldNpcQuickMoveSpeed = 1.2f;
	float3_t m_WorldNpcBatchCenter = {};
	bool_t m_bWorldNpcBatchCenterValid = false;
	f32_t m_fWorldNpcBatchRadius = 10.f;
	f32_t m_fWorldNpcBatchMinimumSpacing = 1.5f;
	uint32_t m_iWorldNpcBatchCount = 8;
	uint32_t m_iWorldNpcBatchSeed = 1;
	bool_t m_bWorldNpcBatchRandomYaw = true;
	bool_t m_bWorldNpcBatchCopySelectedBehavior = false;
	char m_WorldNpcBatchIdPrefix[128] = "npc.batch";
	std::unordered_set<std::string> m_WorldNpcBatchArchetypePool;
	std::vector<WORLD_GAMEPLAY_PLACEMENT> m_WorldNpcBatchDraft;
	uint32_t m_iWorldNpcBatchDraftBaseRevision = 0;
	std::unique_ptr<CWorldSequenceToolPanel> m_pWorldSequenceToolPanel;

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
	/* The staged bake between Preview and Apply. Nothing here has touched
	   disk yet, so discarding it leaves the current navigation untouched. */
	bool_t m_bNavigationBakePreviewReady = false;
	bool_t m_bNavigationBakePreviewLayoutChanged = false;
	uint32_t m_iNavigationBakePreviewWalkable = {};
	NAVGRID_BAKE_RESULT m_NavigationBakePreview;

	NAVIGATION_EDIT_ACTION m_eNavigationEditAction =
		NAVIGATION_EDIT_ACTION::APPLY;
	uint32_t m_iBrushRadius = {};
	bool_t m_bNavigationUsePickedHeight = false;
	/* Cells without a baked surface carry no height, so the overlay has to
	   draw them on the Nav Bounds floor. On a large bake they outnumber the
	   real surface cells and hide it, so they stay off unless asked for. */
	bool_t m_bShowUnresolvedCells = false;

	/* Empty means the Area's base grid. Otherwise the region whose grid id is
	   "<AreaId>.<regionId>"; every navigation path in this tool then points at
	   that grid instead. */
	std::string m_NavigationRegionId;
	/* The Area manifest as loaded, so the combo does not read the file every
	   frame. Rewritten by Commit_NavigationRegionManifest. */
	std::vector<MAP_NAVIGATION_REGION> m_NavigationRegions;
	char m_NewNavigationRegionId[33] = "stage1";
	f32_t m_NewNavigationRegionStepHeight = 1.f;

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
	std::vector<EDITOR_CAMERA_SHOT> m_CameraShots;
	std::string m_strCameraShotBaselineText;
	/* Writer text of the state last loaded or saved. A draft is dirty only
	   when the editor changed it, not when the source used other number text. */
	std::string m_strCameraShotLoadedDocumentText;
	/* Area whose camera document is loaded. A failed Reload of the same Area
	   keeps the draft; a switch to another Area never keeps a stale one. */
	std::string m_strCameraShotAreaId;
	bool_t m_bCameraShotReloadConfirmPending = false;
	size_t m_iSelectedCameraShot = 0u;
	std::string m_CameraShotStatus = "No camera shot document for this Area";
	bool_t m_bCameraShotPreviewActive = false;
	/* Never saved. Closing Map Tool always clears this overlay. */
	bool_t m_bCameraPreviewSurroundingsCleared = false;
	std::vector<uint64_t> m_CameraPreviewSuppressedDeployPlacementIds;
	/* Area cutscenes. Loaded from the same camera document, so a file without
	   them keeps the previous single-shot editing untouched. */
	std::vector<EDITOR_CUTSCENE> m_Cutscenes;
	size_t m_iSelectedCutscene = 0u;
	/* Session state. Only one cutscene previews at a time; its Area is kept so
	   switching Area tears the preview down instead of driving the wrong map. */
	EDITOR_CUTSCENE_STATE m_eCutsceneState = EDITOR_CUTSCENE_STATE::STOPPED;
	std::string m_strCutsceneSessionId;
	std::string m_strCutsceneSessionArea;
	f32_t m_fCutsceneSessionMs = 0.f;
	bool_t m_bCutsceneSoundNaturallyFinished = false;
	bool_t m_bCutsceneSoundSeekRequested = false;
	/* The cut that owned the camera last frame, so a cut change can restart
	   the blend and a gap can hand the camera back exactly once. */
	std::string m_strCutsceneActiveCutId;
	bool_t m_bCutsceneWorldPrepared = false;
	std::string m_CutsceneStatus;
	/* Instances this session started. Kept apart from the prepared flag so a
	   Stop after a partial failure still releases every survivor. */
	std::vector<std::string> m_CutsceneSessionInstanceIds;
	/* Where the session actors came from: the World draft or the published
	   runtime document, shown so a preview is never mistaken for the other. */
	std::string m_CutsceneWorldSource;
	bool_t m_bCutsceneWorldFailed = false;
	bool_t m_bCutsceneWorldPreviewStale = false;
	size_t m_iSelectedCutsceneActor = 0u;
	std::string m_CutsceneSaveStatus;
	/* Taken when an actor field becomes active and restored if the finished
	   edit fails document validation, so a bad value never stays in the draft. */
	std::optional<WORLD_SEQUENCE_INSTANCE> m_CutsceneEditInstanceSnapshot;
	std::optional<WORLD_SEQUENCE_TEMPLATE> m_CutsceneEditTemplateSnapshot;
	int32_t m_iCutsceneSelectedActorKey = 0;
	std::string m_CutsceneActorEditStatus;
	std::string m_CutsceneSubtitleSequenceId;
	std::optional<WORLD_SEQUENCE_SUBTITLE_TRACK> m_CutsceneSubtitleDraft;
	std::string m_CutsceneSoundSequenceId;
	std::optional<WORLD_SEQUENCE_SOUND_TRACK> m_CutsceneSoundDraft;
	/* Integrated cutscene view. The session and its documents stay with their
	   existing owners; only the view state lives here. */
	ICompositionWorkbenchSession* m_pSequenceCompositionSession = nullptr;
	ICompositionWorkbenchSession* m_pValtanCompositionSession = nullptr;
	/* Set for the frame the integrated view opens a session frame. */
	ICompositionWorkbenchSession* m_pHostedCompositionSession = nullptr;
	ICompositionWorkbenchSession* m_pObjectCompositionSession = nullptr;
	bool_t m_bHostingObjectSession = false;
	bool_t m_bIntegratedSequenceDirty = false;
	bool_t m_bIntegratedObjectDirty = false;
	bool_t m_bIntegratedSaveRequested = false;
	std::string m_IntegratedSaveStatus;
	bool_t m_bIntegratedCutsceneView = false;
	bool_t m_bHostingCompositionSession = false;
	/* Camera track timeline view state. Display only: zoom and drag never
	   change an authored value on their own. */
	f32_t m_fCameraTrackZoomPxPerSecond = 120.f;
	/* Captured once per gesture so an accumulated drag delta is applied to
	   the value the key had when the drag started, never to itself. */
	int32_t m_iCameraTrackDragKey = -1;
	int32_t m_iCameraTrackDragOriginMs = 0;
	/* Sorted key indices. A group drag captures every origin once so the
	   accumulated delta is applied to the values the gesture started from. */
	std::vector<int32_t> m_CameraTrackSelection;
	std::vector<int32_t> m_CameraTrackDragOriginsMs;
	bool_t m_bCameraTrackMarqueeActive = false;
	float2_t m_CameraTrackMarqueeStart = {};
	std::string m_CameraStatus = "Open ASSET_TEST with F2";
	/* One camera jump target per authored source sublevel of the active
	Area, rebuilt from the committed placements. An Area whose placements
	carry no sublevel simply produces none, which is what keeps the number
	shortcuts inert outside the Areas that need them. */
	struct EDITOR_SUBLEVEL_JUMP final
	{
		std::string label;
		float3_t center = {};
		f32_t radius = 0.f;
		size_t placementCount = 0u;
	};
	vector<EDITOR_SUBLEVEL_JUMP> m_EditorSublevelJumps;
};

NS_END
