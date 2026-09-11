#pragma once

#include "CompositionResourceTree.h"
#include "CompositionAnimationResource.h"
#include "CompositionWorkbenchSession.h"
#include "KoukuSaydonCompositionDocument.h"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	/* One-shot transport command for the local composition preview. MainApp
	   consumes it and forwards it to the real-CModel preview owner. */
	enum class KOUKU_PATTERN_SELECTION : std::uint8_t { GATE, FOLDER, BUNDLE, PATTERN };

	enum class KOUKU_PREVIEW_TRANSPORT : std::uint8_t
	{
		NONE,
		PAUSE,
		RESUME,
		STOP,
		SEEK
	};

	/* Immutable snapshot of the preview clock handed back by MainApp each
	   frame. The Workbench draws its playhead from this and owns no model. */
	struct KOUKU_PREVIEW_STATE final
	{
		bool_t bPlaying = false;
		bool_t bPaused = false;
		std::uint32_t iClockMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::string strPatternId;
		std::string strStatus;
	};

	/* One authored world sequence instance of the arena, listed for the WORLD
	   lane. MainApp reads it from the arena level's loaded document; the first
	   bound map placement lets a roulette Logic copy its centre. */
	struct KOUKU_WORLD_SEQUENCE_RESOURCE final
	{
		std::string strInstanceId;
		std::string strDisplayName;
		std::uint32_t iDurationMs = 0u;
		bool_t bHasBoundPlacement = false;
		f32_t fBoundX = 0.f;
		f32_t fBoundZ = 0.f;
		std::string strObjectResourceId;
		bool_t bEnabled = true;
		bool_t bSupportsPlacement = false;
		std::string strObjectDisplayName;
		std::string strAnchorKind = "WORLD";
		bool_t bDefaultMotion = false;
		std::vector<std::string> AnimationClips;
		// Authored emission rows of this Motion (1 for seeded emitters); Box Detail offers the row index.
		std::uint32_t iEmissionCount = 1u;
	};

	struct KOUKU_PRESENTATION_PREVIEW_REQUEST final
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE Resource;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE Occurrence;
		// Isolated authoring placement preview: all placed Objects, without boss or Logic playback.
		std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE> WorldBoxes;
		// A live edit may retain an existing preview only when this exact box is active.
		std::string strEditedOccurrenceId;
	};

	struct KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST final
	{
		std::string strPatternId;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE Occurrence;
	};

	/* K-only authoring session and Stage/Animation lane editor. It owns no socket
	   or runtime executor; typed preview/server-play requests are consumed by
	   MainApp and routed to their dedicated tools/services. */
	class CKoukuSaydonActionWorkbench final : public ICompositionWorkbenchSession
	{
	public:
		explicit CKoukuSaydonActionWorkbench(bool sequenceWorkspace = false);
		~CKoukuSaydonActionWorkbench();

		void Open();
		bool_t Select_ActorProfile(std::string_view actorProfileId, std::string& outStatus);
		[[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
		void Render();
		void Begin_WorkbenchFrame() override;
		void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) override;
		void End_WorkbenchFrame() override;
		void Tick_Background() { Poll_PublishProcess(); }
		bool_t Consume_ProductInventoryRefreshRequest() {
			const bool_t requested = m_bProductInventoryRefreshRequested;
			m_bProductInventoryRefreshRequested = false;
			return !m_bSequenceWorkspace && requested;
		}
		bool_t Consume_PresentationPreviewRequest(KOUKU_PRESENTATION_PREVIEW_REQUEST& outRequest);
		/* MainApp supplies admitted camera/audio rows from the existing readers;
		   a failed refresh preserves the prior complete list and its status. */
		void Set_PresentationResources(
			std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> resources,
			std::string status);
		bool_t Consume_PresentationResourceRefreshRequest();
		bool Can_AppendCompositionAnimationResource(
			const COMPOSITION_ANIMATION_RESOURCE& resource, bool asNewStage,
			std::string& outStatus) const override;
		bool Append_CompositionAnimationResource(
			const COMPOSITION_ANIMATION_RESOURCE& resource, bool asNewStage,
			std::string& outStatus) override;
		bool Consume_ResourceRefreshRequest() {
			const bool requested = m_bResourceRefreshRequested;
			m_bResourceRefreshRequested = false;
			return requested;
		}
		void Set_ModelResources(std::vector<COMPOSITION_ANIMATION_RESOURCE> resources,
			std::string status);
		void Set_SequenceResources(std::vector<COMPOSITION_ANIMATION_SEQUENCE_RESOURCE> resources,
			std::string status, bool_t succeeded = true) {
			if (succeeded) m_SequenceResources = std::move(resources);
			m_strSequenceResourceStatus = std::move(status);
			m_bResourceTreeDirty = true;
		}
		void Set_WorldSequenceResources(
			std::vector<KOUKU_WORLD_SEQUENCE_RESOURCE> resources, std::string status)
		{
			m_WorldSequenceResources = std::move(resources);
			m_strWorldSequenceResourceStatus = std::move(status);
		}
		void Set_RenderingProfileResources(
			std::vector<std::string> profileIds, std::string status)
		{
			m_RenderingProfileIds = std::move(profileIds);
			m_strRenderingProfileResourceStatus = std::move(status);
		}
		void Select_WorkbenchBoss(COMPOSITION_WORKBENCH_BOSS boss) override;
		bool_t Consume_PreviewTransportRequest(
			KOUKU_PREVIEW_TRANSPORT& outTransport,
			std::uint32_t& outSeekMs);
		void Set_PreviewState(const KOUKU_PREVIEW_STATE& state) {
			if (!state.strStatus.empty() && (m_bPreviewResultStatusPending ||
				state.strStatus != m_PreviewState.strStatus))
				m_strStatus = state.strStatus;
			m_bPreviewResultStatusPending = false;
			m_PreviewState = state;
		}
		[[nodiscard]] const std::string& Get_Status() const noexcept { return m_strStatus; }
		[[nodiscard]] const KOUKU_PREVIEW_STATE& Get_PreviewState() const noexcept {
			return m_PreviewState;
		}


		bool_t Reload(std::string& outStatus);
		bool_t Save(std::string& outStatus);
		// Save is source-only; publication always considers the complete saved tree.
		bool_t Publish_AllPatterns(std::string& outStatus);
		[[nodiscard]] bool_t Is_PublishRunning() const noexcept {
			return nullptr != m_hPublishProcess;
		}
		bool_t Validate_Draft(std::string& outStatus) const;
		[[nodiscard]] bool_t Has_Composition() const noexcept {
			return m_bHasDraft;
		}
		[[nodiscard]] bool_t Is_Dirty() const noexcept { return m_bDirty || !m_StagedPresentationGeometry.empty(); }
		[[nodiscard]] std::uint64_t Get_DraftGeneration() const noexcept { return m_iDraftGeneration; }
		[[nodiscard]] const KOUKU_SAYDON_COMPOSITION_DOCUMENT&
			Get_Composition() const noexcept { return m_Draft; }
		[[nodiscard]] const std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN>&
			Get_AuthoringPatterns() const noexcept { return m_Draft.Patterns; }

		[[nodiscard]] const std::string& Get_SelectedPatternId() const noexcept {
			return m_strSelectedPatternId;
		}
		bool_t Select_PatternById(
			std::string_view patternId,
			std::string& outStatus);
		bool_t Consume_AnimationPreviewRequest(
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& outRequest);
		// Play restarts at the end; paused scrubbing keeps the exact endpoint pose.
		bool_t Request_PatternPreview(std::string_view patternId,
			std::uint32_t startClockMs, std::string& outStatus, bool_t startPaused = false);
		bool_t Request_PreviewPause();
		bool_t Request_PatternScrub(std::string_view patternId, std::uint32_t clockMs, std::string& outStatus);
		bool_t Request_BundleScrub(std::uint32_t clockMs);
		// Detail Preview keeps its actor animation and starts at this edited Bone Collider's window.
		bool_t Request_ColliderBoxPreview(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value, std::string& outStatus);
		// Geometry-only draft overlay; active previews retain their clock, actors and sessions.
		bool_t Request_PresentationGeometryPreview(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value, std::string& outStatus);
		bool_t Consume_PresentationGeometryPreviewRequest(KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST& outRequest);
		void Cancel_PresentationGeometryPreview(bool_t discardStagedGeometry = true);
		bool_t Consume_PatternPreviewRequest(
			KOUKU_SAYDON_COMPOSITION_PATTERN& outPattern,
			std::uint32_t& outStartClockMs,
			bool_t& outStartPaused,
			std::string& outTargetAssetName);
		bool_t Consume_BundlePreviewRequest(std::string& bundleId, std::uint32_t& clockMs, bool_t& paused);
		bool_t Consume_BundleServerPlayRequest(std::string& bundleId, std::uint32_t& revision);
		bool_t Select_BundleById(std::string_view bundleId, std::string& status);
		bool_t Consume_ServerPlayRequest(
			std::string& outPatternId,
			std::uint32_t& outSourceRevision);

		// Create Pattern's Parent/Bundle controls validate their destination before editing session state.
		bool_t Set_PatternCreationDestination(std::string_view folderId,
			std::string_view bundleId, std::string& outStatus);
		bool_t Create_Pattern(
			std::string_view displayName,
			std::string_view category,
			std::string& outPatternId,
			std::string& outStatus);
		bool_t Delete_Pattern(
			std::string_view patternId,
			std::string& outStatus);
		bool_t Rename_Pattern(
			std::string_view patternId,
			std::string_view displayName,
			std::string& outStatus);
		bool_t Set_PatternFolder(std::string_view patternId,
			std::string_view folderId, std::string& outStatus);
		bool_t Set_PatternCategory(
			std::string_view patternId,
			std::string_view category,
			std::string& outStatus);
		bool_t Set_PatternAuthoringStatus(
			std::string_view patternId,
			std::string_view authoringStatus,
			std::string& outStatus);

		bool_t Add_Stage(
			std::string_view patternId,
			std::string_view stageKind,
			std::uint32_t durationMs,
			std::string& outStageId,
			std::string& outStatus);
		bool_t Append_AnimationAsStage(
			std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
			std::string& outStageId,
			std::string& outOccurrenceId,
			std::string& outStatus);
		/* Every clip slot of one extracted action, one Stage per source stage
		   with its slots laid out back to back. */
		bool_t Append_ActionAsStages(
			std::string_view patternId,
			std::string_view profileId,
			std::uint32_t sourceActionId,
			std::string& outStatus);
		/* The same slots appended after the target Stage's last box; the Stage
		   clock grows to hold them. */
		bool_t Append_ActionToStage(
			std::string_view patternId,
			std::string_view stageId,
			std::string_view profileId,
			std::uint32_t sourceActionId,
			std::string& outStatus);
		bool_t Delete_Stage(
			std::string_view patternId,
			std::string_view stageId,
			std::string& outStatus);
		// Validate every stable ID before deleting the selection in one draft commit.
		bool_t Delete_TimelineSelection(
			std::string_view patternId,
			const std::vector<std::string>& stageIds,
			const std::vector<std::string>& occurrenceIds,
			std::string& outStatus);
		// All lane occurrence IDs share this atomic copy command; Stage children are copied only once.
		// Mixed blocks preserve relative clocks, splice later lanes and remap owned stable references.
		bool_t Duplicate_TimelineSelection(
			std::string_view patternId,
			const std::vector<std::string>& stageIds,
			const std::vector<std::string>& occurrenceIds,
			std::string& outStatus);
		// Total lifetime is the sum of Stage clocks; only the final Stage is resized.
		bool_t Set_PatternDuration(std::string_view patternId,
			std::uint32_t durationMs, std::string& outStatus);
		bool_t Move_Stage(
			std::string_view patternId,
			std::string_view stageId,
			int32_t direction,
			std::string& outStatus);
		/* Left/Right and Earlier/Later reorder selected Stages and animation
		   owners together. Stable selection and internal order are preserved;
		   the entire move is rejected at the corresponding Pattern edge. */
		bool_t Move_SelectedStage(
			std::string_view patternId,
			int32_t direction,
			std::string& outStatus);
		bool_t Set_StageDuration(
			std::string_view patternId,
			std::string_view stageId,
			std::uint32_t durationMs,
			std::string& outStatus);
		bool_t Set_StageKind(
			std::string_view patternId,
			std::string_view stageId,
			std::string_view stageKind,
			std::string& outStatus);

		bool_t Set_StageRetargetOnEnter(
			std::string_view patternId,
			std::string_view stageId,
			bool_t retargetOnEnter,
			std::string& outStatus);

		bool_t Bind_Animation(
			std::string_view patternId,
			std::string_view targetStageId,
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
			std::uint32_t startOffsetMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Move_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startOffsetMs,
			std::string& outStatus);
		bool_t Trim_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t sourceStartMs,
			std::uint32_t playMs,
			std::string& outStatus);
		/* Rate and end policy are the box's own playback values. EXACT is
		   rejected when it would outrun the native clip; drag/trim convert such
		   a window to HOLD_LAST_POSE instead of silently truncating it. */
		bool_t Set_AnimationPlayback(
			std::string_view patternId,
			std::string_view occurrenceId,
			f32_t playRate,
			std::string_view endPolicy,
			std::string& outStatus);
		bool_t Set_AnimationBlend(std::string_view patternId, std::string_view occurrenceId,
			std::uint32_t blendInMs, std::string& outStatus);
		bool_t Duplicate_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Delete_Animation(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);
		bool_t Move_AnimationToStage(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string_view targetStageId,
			std::uint32_t startOffsetMs,
			std::string& outStatus);

		/* Logic catalog. A definition is named once in Resources; a box places it
		   on one Pattern with a pattern-relative window. Values and the Server
		   consumer arrive in a later slice, so a PRODUCT Pattern may not own boxes. */
		bool_t Create_Logic(
			std::string_view displayName,
			std::string_view logicType,
			std::string& outLogicId,
			std::string& outStatus);
		bool_t Delete_Logic(
			std::string_view logicId,
			std::string& outStatus);
		bool_t Append_LogicBox(
			std::string_view patternId,
			std::string_view logicId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_LogicBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outStatus);
		bool_t Delete_LogicBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);
		bool_t Set_LogicBoxHold(std::string_view patternId, std::string_view occurrenceId,
			std::string_view holdOccurrenceId, std::string& outStatus);
		/* Replaces one outcome slot of a DURATION box with an ordered list of up
		   to four RESULT Logics; an empty list clears the slot. The box owns the
		   wiring, so the same definition may succeed into different results on
		   different Patterns. */
		bool_t Set_LogicBoxOutcomes(
			std::string_view patternId,
			std::string_view occurrenceId,
			KOUKU_SAYDON_OUTCOME_SLOT slot,
			const std::vector<std::string>& resultLogicIds,
			std::string& outStatus);
		/* Replaces the typed judgement or outcome values of one definition; the
		   identity, name and type stay. */
		bool_t Set_LogicDefinitionValues(
			std::string_view logicId,
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& values,
			std::string& outStatus);

		// Apply only this debug flag, retaining all uncommitted Box Detail values.
		bool_t Connect_ColliderLogic(const std::string& patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
			const std::string& logicId, std::string& outStatus);
		// Apply the definition, exact Logic window and Collider link as one authoring transaction.
		bool_t Set_ColliderLogicValues(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& values, std::string& outStatus);
		bool_t Set_PresentationBox(std::string_view patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& value, std::string& outStatus);
		bool_t Set_PresentationBoxDebugRender(std::string_view patternId,
			std::string_view occurrenceId, bool_t visible, std::string& outStatus);

		// Atomically connect ENTER_AREA and a damage RESULT while retaining other outcome slots.
		bool_t Set_ColliderTriggerDamage(const std::string& patternId,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence,
			std::uint32_t percent, std::string& outStatus);

		/* Summon catalog. A definition is only a name today; a box places it on
		   one Pattern with spawn time (startMs) and lifetime (durationMs). */
		bool_t Create_Summon(
			std::string_view displayName,
			std::string& outSummonId,
			std::string& outStatus);
		bool_t Delete_Summon(
			std::string_view summonId,
			std::string& outStatus);
		bool_t Append_SummonBox(
			std::string_view patternId,
			std::string_view summonId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_SummonBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outStatus);
		bool_t Delete_SummonBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);

		/* World catalog: a definition names one authored world sequence instance
		   of the arena; a box starts it on the pattern clock at a playback speed. */
		void Set_WorldPlacementResolver(std::function<bool_t(KOUKU_SAYDON_WORLD_PLACEMENT&, std::string&)> resolver)
		{ m_WorldPlacementResolver = std::move(resolver); }
		bool_t Append_WorldResource(std::string_view instanceId, std::string& outStatus, bool_t asObject = false);
		bool_t Append_WorldObject(std::string_view objectResourceId, std::string& outStatus);
		bool_t Set_WorldBoxPlacement(std::string_view patternId, std::string_view occurrenceId,
			const KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& outStatus);
		bool_t Place_WorldBoxNearCharacter(std::string_view patternId, std::string_view occurrenceId,
			std::string& outStatus);
		bool_t Create_World(
			std::string_view displayName,
			std::string_view sequenceInstanceId,
			std::string& outWorldId,
			std::string& outStatus);
		bool_t Set_WorldCompanionEffect(std::string_view worldId,
			std::string_view resourceId, std::string& outStatus);
		bool_t Delete_World(
			std::string_view worldId,
			std::string& outStatus);
		bool_t Append_WorldBox(
			std::string_view patternId,
			std::string_view worldId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_WorldBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			f32_t playbackSpeed,
			std::string& outStatus);
		bool_t Delete_WorldBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);

		/* Scene Profile catalog: a definition names one rendering profile; a box
		   applies it for its window and blends in over blendMs. */
		bool_t Create_SceneProfile(
			std::string_view displayName,
			std::string_view renderingProfileId,
			std::string& outSceneProfileId,
			std::string& outStatus);
		bool_t Delete_SceneProfile(
			std::string_view sceneProfileId,
			std::string& outStatus);
		bool_t Append_SceneProfileBox(
			std::string_view patternId,
			std::string_view sceneProfileId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::string& outOccurrenceId,
			std::string& outStatus);
		bool_t Set_SceneProfileBoxWindow(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::uint32_t startMs,
			std::uint32_t durationMs,
			std::uint32_t blendMs,
			std::string& outStatus);
		bool_t Delete_SceneProfileBox(
			std::string_view patternId,
			std::string_view occurrenceId,
			std::string& outStatus);

	private:
		enum class RENAME_TARGET : std::uint8_t
		{
			PATTERN, FOLDER, BUNDLE, LOGIC, SUMMON, WORLD, SCENE_PROFILE, PRESENTATION
		};
		bool_t Rename_Item(RENAME_TARGET target, std::string_view id,
			std::string_view displayName, std::string& outStatus);
		bool_t Render_RenameControl(RENAME_TARGET target, std::string_view id,
			std::string_view displayName);
		void Render_PresentationAnchor(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence, bool_t effect);

		/* One browsable extracted action: indices into the immutable reference
		   set snapshot that Reload/Save replaced last. */
		struct RESOURCE_ACTION_LEAF final
		{
			std::size_t iDocument = 0u;
			std::size_t iAction = 0u;
			bool_t bSequence = false;
		};

		bool_t Commit_Candidate(
			KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate,
			std::string_view successStatus,
			std::string& outStatus);
		bool_t Is_AppendAdmitted(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& source,
			std::string& outStatus) const;
		bool_t Resolve_NativeClipMs(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
			std::uint32_t& outNativeMs) const;
		bool_t Validate_SourceStart(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
			std::string& outStatus) const;
		void Queue_AnimationPreview(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence);
		bool_t Normalize_EndPolicyForWindow(
			KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE& occurrence,
			std::string& outNote) const;
		void Queue_ModelResourcePreview(const COMPOSITION_ANIMATION_RESOURCE& resource);
		void Queue_SequencePreview(const COMPOSITION_ANIMATION_SEQUENCE_RESOURCE& sequence);
		void Queue_ResourcePatternPreview(KOUKU_SAYDON_COMPOSITION_PATTERN pattern,
			const std::string& targetAssetName);
		void Queue_SlotPreview(
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action,
			const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& stage,
			const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& slot);
		void Queue_ActionPreview(
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& action,
			const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE* selectedStage);
		void Rebuild_ResourceTree();
		void Normalize_Selection();
		void Synchronize_EditorFields();
		void Render_Toolbar();
		void Render_PatternsAndResources();
		void Select_Hierarchy(KOUKU_PATTERN_SELECTION kind, std::string_view id);
		void Render_HierarchyDetails();
		void Render_BundleTimeline();
		void Render_BundleTransport();
		bool_t Request_BundlePreview(std::uint32_t clockMs, bool_t paused = false);
		bool_t Create_Hierarchy(bool_t bundle);
		bool_t Link_BundlePattern(std::string_view patternId);
		void Render_BundleCommonDetails();
		void Render_ResourceTree();
		void Render_AnimationResources();
		void Render_LogicResources();
		void Render_LogicDefinitionValues(
			const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic,
			std::string_view colliderPatternId = {},
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* collider = nullptr);
		void Render_LogicBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		bool_t Render_LogicOutcomeSlots(const std::string& patternId, const std::string& occurrenceId);
		void Render_SummonResources();
		void Render_SummonBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_PresentationResources(KOUKU_SAYDON_PRESENTATION_KIND kind);
		void Render_CameraAuthoring(std::string_view shotId);
		void Render_PresentationBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		bool_t Create_PresentationResource(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source,
			std::string_view displayName, std::string& outStatus);
		bool_t Append_PresentationSource(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& source,
			std::string& outStatus);
		bool_t Append_PresentationBox(std::string_view resourceId, std::string& outStatus);
		bool_t Append_PresentationCandidate(KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate,
			std::string_view resourceId, std::string& outStatus);
		bool_t Delete_PresentationBox(std::string_view patternId, std::string_view occurrenceId, std::string& outStatus);
		void Queue_WorldBoxPreview(std::string_view patternId, std::string_view occurrenceId);
		void Queue_PresentationPreview(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
			const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* occurrence = nullptr);
		void Render_WorldResources();
		bool_t Render_WorldCompanionSelector(const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world);
		void Render_WorldBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_SceneProfileResources();
		void Render_SceneProfileBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_ResourcesWindow();
		void Render_Timeline();
		void Clear_TimelineSelection();
		void Select_TimelineBox(const std::string& stageId,
			const std::string& occurrenceId, bool_t toggle);
		void Render_Transport();
		void Stop_Preview();
		void Render_Details();
		void Render_ReloadConfirmation();
		void Poll_PublishProcess();

	private:
		const bool m_bSequenceWorkspace;
		CKoukuSaydonCompositionDocument m_Document;
		KOUKU_SAYDON_ACTION_REFERENCE_SET m_ResourceReferences;
		std::vector<COMPOSITION_ANIMATION_RESOURCE> m_ModelResources;
		std::vector<COMPOSITION_ANIMATION_SEQUENCE_RESOURCE> m_SequenceResources;
		std::string m_strSequenceResourceStatus;
		std::string m_strSelectedSequenceResourceId;
		std::string m_strSelectedResourceTargetAsset;
		std::string m_strResourceStatus;
		char m_ResourceSearch[128]{};
		bool m_bResourcesOpen = true;
		bool m_bResourceRefreshRequested = true;
		COMPOSITION_RESOURCE_TREE_NODE m_ResourceTree;
		COMPOSITION_RESOURCE_TREE_NODE m_PhysicalResourceTree;
		std::vector<RESOURCE_ACTION_LEAF> m_ResourceLeaves;
		std::string m_strResourceTreeQuery;
		bool_t m_bResourceTreeDirty = true;
		std::string m_strSelectedResourceProfileId;
		std::uint32_t m_iSelectedResourceActionId = 0u;
		// Expansion belongs to this session and survives search/catalog rebuilds by stable source ID.
		std::string m_strExpandedResourceActionId;
		std::string m_strExpandedResourceStageId;
		// Resources family tab and Logic authoring session state; none of it is document data.
		int32_t m_iSelectedResourceCategory = 0;
		int32_t m_iNewLogicType = 0;
		int32_t m_iNewLogicBoxDurationMs = 1000;
		char_t m_NewLogicName[256]{};
		std::string m_strSelectedLogicId;
		std::string m_strSelectedLogicOccurrenceId;
		int32_t m_iLogicBoxStartMs = 0;
		int32_t m_iLogicBoxDurationMs = 1000;
		// Summon authoring session state; a new box defaults to the remaining Pattern lifetime.
		char_t m_NewSummonName[256]{};
		std::string m_strSelectedSummonId;
		std::string m_strSelectedSummonOccurrenceId;
		int32_t m_iNewSummonBoxDurationMs = 1000;
		bool_t m_bNewSummonBoxToPatternEnd = true;
		int32_t m_iSummonBoxStartMs = 0;
		int32_t m_iSummonBoxDurationMs = 1000;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> m_PresentationResourceInventory;
		std::string m_strPresentationResourceStatus;
		std::string m_strSelectedPresentationResourceId;
		std::string m_strSelectedPresentationSourceId;
		int32_t m_iLightResourceCategory = 0;
		int32_t m_iEffectResourceVersion = 0;
		std::string m_strExpandedV1EffectId;
		std::string m_strV1ElementResourceStatus;
		std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> m_V1ElementResources;
		char_t m_PresentationResourceSearch[128]{};
		// Rename text stays local until Apply; stable IDs and references never change.
		std::string m_strRenameItemId;
		char_t m_RenameDisplayName[256]{};
		std::string m_strSelectedPresentationOccurrenceId;
		char_t m_NewPresentationName[256]{};
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE m_PresentationBoxEdit;
		std::string m_strColliderExecutionType = "DURATION";
		std::string m_strColliderExecutionEditId;
		std::string m_strColliderLogicDefinitionId;
		int32_t m_iColliderDamagePercent = 10;
		bool_t m_bColliderDamageDirty = false;

		std::vector<KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST> m_PendingPresentationGeometryPreviews;
		std::vector<KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST> m_StagedPresentationGeometry;
		std::string m_strPresentationGeometryPreviewPatternId;
		std::string m_strPresentationGeometryPreviewOccurrenceId;
		KOUKU_PRESENTATION_PREVIEW_REQUEST m_PendingPresentationPreviewRequest;
		bool_t m_bPresentationPreviewRequestPending = false;
		bool_t m_bPresentationResourceRefreshRequested = true;
		bool_t m_bProductInventoryRefreshRequested = false;
		// World lane session state: the arena's sequence list and the box being edited.
		std::vector<KOUKU_WORLD_SEQUENCE_RESOURCE> m_WorldSequenceResources;
		std::function<bool_t(KOUKU_SAYDON_WORLD_PLACEMENT&, std::string&)> m_WorldPlacementResolver;
		bool_t Stage_NewWorldPlacement(std::string_view instanceId,
			std::optional<KOUKU_SAYDON_WORLD_PLACEMENT>& outPlacement, std::string& outStatus) const;
		std::string m_strWorldSequenceResourceStatus;
		char_t m_NewWorldName[256]{};
		std::string m_strNewWorldObjectId;
		char m_WorldObjectFilter[128]{};
		std::string m_strSelectedWorldId;
		std::string m_strSelectedWorldOccurrenceId;
		int32_t m_iWorldBoxStartMs = 0;
		int32_t m_iWorldBoxDurationMs = 1000;
		f32_t m_fWorldBoxPlaybackSpeed = 1.f;
		// Scene Profile lane session state: the rendering profile list and the box being edited.
		std::vector<std::string> m_RenderingProfileIds;
		std::string m_strRenderingProfileResourceStatus;
		char_t m_NewSceneProfileName[256]{};
		std::string m_strNewSceneProfileRenderingId;
		std::string m_strSelectedSceneProfileId;
		std::string m_strSelectedSceneProfileOccurrenceId;
		bool_t m_bNewSceneProfileBoxToPatternEnd = true;
		int32_t m_iNewSceneProfileBoxDurationMs = 1000;
		int32_t m_iSceneProfileBoxStartMs = 0;
		int32_t m_iSceneProfileBoxDurationMs = 1000;
		int32_t m_iSceneProfileBoxBlendMs = 500;
		// Typed Logic values under edit; committed to the draft by Apply Values.
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION m_LogicValueDraft;
		std::string m_strLogicValueDraftId;
		// Which gate the shell selected; only the pattern list header and model filter follow it.
		std::string m_strBossVariantLabel;

		KOUKU_SAYDON_COMPOSITION_DOCUMENT m_Draft;
		KOUKU_PATTERN_SELECTION m_ePatternSelection = KOUKU_PATTERN_SELECTION::GATE;
		std::string m_strSelectedGateId = "GATE1";
		std::string m_strModelViewProfile;
		std::string m_strSelectedFolderId;
		std::string m_strSelectedBundleId;
		std::string m_strCreateFolderId;
		std::string m_strCreateBundleId;
		std::string m_strCreateActorProfileId = "MN_RPCT_05";
		std::string m_strBundleReturnId;
		std::string m_strBundleDragMemberId;
		char m_NewFolderName[256]{};
		char m_NewBundleName[256]{};
		char m_HierarchyName[256]{};
		bool_t m_bBundlePreviewRequestPending = false;
		std::string m_strPendingBundlePreviewId;
		std::string m_strPendingBundleServerId;
		std::uint32_t m_iPendingBundleServerRevision = 0;
		std::string m_strSelectedPatternId;
		std::string m_strSelectedActorProfileId = "MN_RPCT_05";
		std::string m_strSelectedStageId;
		std::string m_strSelectedOccurrenceId;
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE m_SelectedResource;
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE m_PendingPreviewRequest;
		KOUKU_SAYDON_COMPOSITION_PATTERN m_PendingPatternPreview;
		std::string m_strPendingPreviewTargetAsset;
		KOUKU_PREVIEW_STATE m_PreviewState;
		KOUKU_PREVIEW_TRANSPORT m_ePendingTransport = KOUKU_PREVIEW_TRANSPORT::NONE;
		std::uint32_t m_iPendingSeekMs = 0u;
		std::uint32_t m_iCursorMs = 0u;
		std::string m_strCursorPatternId;
		std::uint32_t m_iPendingPreviewStartMs = 0u;
		bool_t m_bPendingPreviewStartPaused = false;
		std::string m_strStatus;
		std::filesystem::path m_PublishDiagnosticPath;
		void* m_hPublishProcess = nullptr;
		std::uint64_t m_iPublishStartedAtMilliseconds = 0u;
		std::string m_strPendingServerPlayPatternId;
		std::uint32_t m_iPendingServerPlaySourceRevision = 0u;
		std::uint64_t m_iDraftGeneration = 0u;
		std::uint32_t m_iDragOriginOffsetMs = 0u;
		std::uint32_t m_iDragOriginSourceMs = 0u;
		std::uint32_t m_iDragOriginPlayMs = 0u;
		int m_iTimelineDragMode = 0;
		std::string m_strTimelineSelectionPatternId;
		std::vector<std::string> m_TimelineSelectedStageIds;
		std::vector<std::string> m_TimelineSelectedOccurrenceIds;
		bool_t m_bTimelineMarqueeActive = false;
		f32_t m_fTimelineMarqueeStartX = 0.f;
		f32_t m_fTimelineMarqueeStartY = 0.f;
		f32_t m_fPixelsPerSecond = 90.f;
		char_t m_PatternName[256]{};
		char_t m_NewPatternName[256]{};
		int m_iNewPatternCategory = 1;
		int32_t m_iPatternDurationMs = 0;
		int32_t m_iNewStageDurationMs = 1000;
		int32_t m_iOccurrenceStartOffsetMs = 0;
		int32_t m_iOccurrenceSourceStartMs = 0;
		int32_t m_iOccurrencePlayMs = 1;
		f32_t m_fOccurrencePlayRate = 1.f;
		int32_t m_iOccurrenceEndPolicy = 0;
		int32_t m_iOccurrenceBlendInMs = 0;
		int32_t m_iSelectedNewStageKind = 0;
		bool_t m_bOpen = true;
		bool_t m_bSharedWorkspaceActive = false;
		bool_t m_bLoadAttempted = false;
		bool_t m_bHasDraft = false;
		bool_t m_bDirty = false;
		bool_t m_bTimelineMaximized = false;
		bool_t m_bFitRequested = true;
		bool_t m_bReloadConfirmationRequested = false;
		bool_t m_bHasSelectedResource = false;
		bool_t m_bPreviewRequestPending = false;
		bool_t m_bPatternPreviewRequestPending = false;
		// Show the next consumed start result even when its diagnostic repeats.
		bool_t m_bPreviewResultStatusPending = false;
		bool_t m_bServerPlayRequestPending = false;
	};
}
