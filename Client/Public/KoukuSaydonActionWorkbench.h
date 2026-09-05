#pragma once

#include "CompositionResourceTree.h"
#include "CompositionAnimationResource.h"
#include "CompositionWorkbenchSession.h"
#include "KoukuSaydonCompositionDocument.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	/* One-shot transport command for the local composition preview. MainApp
	   consumes it and forwards it to the real-CModel preview owner. */
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

	/* K-only authoring session and Stage/Animation lane editor. It owns no socket
	   or runtime executor; typed preview/server-play requests are consumed by
	   MainApp and routed to their dedicated tools/services. */
	class CKoukuSaydonActionWorkbench final : public ICompositionWorkbenchSession
	{
	public:
		CKoukuSaydonActionWorkbench() = default;
		~CKoukuSaydonActionWorkbench();

		void Open();
		bool_t Select_ActorProfile(std::string_view actorProfileId, std::string& outStatus);
		[[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
		void Render();
		void Begin_WorkbenchFrame() override;
		void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) override;
		void End_WorkbenchFrame() override;
		void Tick_Background() { Poll_PublishProcess(); }
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
		bool_t Consume_PreviewTransportRequest(
			KOUKU_PREVIEW_TRANSPORT& outTransport,
			std::uint32_t& outSeekMs);
		void Set_PreviewState(const KOUKU_PREVIEW_STATE& state) {
			if (!state.strStatus.empty() && state.strStatus != m_PreviewState.strStatus)
				m_strStatus = state.strStatus;
			m_PreviewState = state;
		}


		bool_t Reload(std::string& outStatus);
		bool_t Save(std::string& outStatus);
		bool_t Publish_Product(std::string& outStatus);
		[[nodiscard]] bool_t Is_PublishRunning() const noexcept {
			return nullptr != m_hPublishProcess;
		}
		bool_t Validate_Draft(std::string& outStatus) const;
		[[nodiscard]] bool_t Has_Composition() const noexcept {
			return m_bHasDraft;
		}
		[[nodiscard]] bool_t Is_Dirty() const noexcept { return m_bDirty; }
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
		// Preview the current draft at its cursor; a cursor at/past the end restarts at zero.
		bool_t Request_PatternPreview(std::string_view patternId,
			std::uint32_t startClockMs, std::string& outStatus);
		bool_t Consume_PatternPreviewRequest(
			KOUKU_SAYDON_COMPOSITION_PATTERN& outPattern,
			std::uint32_t& outStartClockMs,
			bool_t& outStartPaused,
			std::string& outTargetAssetName);
		bool_t Consume_ServerPlayRequest(
			std::string& outPatternId,
			std::uint32_t& outSourceRevision);

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
		// Copy selected parents once; standalone boxes keep their Stage and time window.
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
		/* Wires one outcome slot of a DURATION box to a RESULT Logic; an empty
		   resultLogicId clears the slot. The box owns the wiring, so the same
		   definition may succeed into different results on different Patterns. */
		bool_t Set_LogicBoxOutcome(
			std::string_view patternId,
			std::string_view occurrenceId,
			bool_t success,
			std::string_view resultLogicId,
			std::string& outStatus);

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

	private:
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
		void Render_ResourceTree();
		void Render_AnimationResources();
		void Render_LogicResources();
		void Render_LogicBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
		void Render_SummonResources();
		void Render_SummonBoxDetails(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern);
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

		KOUKU_SAYDON_COMPOSITION_DOCUMENT m_Draft;
		std::string m_strSelectedPatternId;
		std::string m_strSelectedActorProfileId = "MN_RPCZ_00";
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
		int32_t m_iPatternDurationMs = 0;
		int32_t m_iNewStageDurationMs = 1000;
		int32_t m_iOccurrenceStartOffsetMs = 0;
		int32_t m_iOccurrenceSourceStartMs = 0;
		int32_t m_iOccurrencePlayMs = 1;
		f32_t m_fOccurrencePlayRate = 1.f;
		int32_t m_iOccurrenceEndPolicy = 0;
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
		bool_t m_bServerPlayRequestPending = false;
	};
}
