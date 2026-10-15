#pragma once

// 이 헤더는 Workbench 세션의 선택·미적용 입력·표시 cache를 선언한다.
// BalanceTool의 gameplay draft와 UI의 값 복사본을 구분해야 Reload·Undo·여러 pane의 수명을 설명할 수 있다.
// Client/Engine 공통 타입과 매크로는 아래 선언의 bool_t·f32_t·NS_BEGIN 등에 필요하다.
#include "Client_Defines.h"
#include "Engine_Defines.h"
// graph는 Boss Pattern 경로를 화면에 투영한 값이며 Server 실행 graph의 소유권을 가져오지 않는다.
// Animation_Tool의 중첩 COMPOSITION_SEQUENCE_VIEW와 BoneEditor 값 멤버는 완전한 타입 정의가 필요하다.
#include "ActionCompositionGraphModel.h"
#include "Animation_Tool.h"
#include "BoneAnimationWorkbench.h"
// resource tree는 목록 표시용 분류이고 session은 공용 pane·명령·전달 인터페이스다.
// Effect 문서·resource key·Undo 이력은 아래 값 멤버와 요청 타입에 사용한다.
#include "CompositionResourceTree.h"
#include "CompositionWorkbenchSession.h"
#include "EffectV2_Document.h"
#include "EditorUndoHistory.h"
#include "EffectResourceCatalog.h"
// 저장 Pattern·Sound·Shake의 읽기 모델과 source 기준본을 값으로 보관한다.
// ValtanViewAdmission은 표시 가능 여부와 변경 가능 여부를 나누는 세션 상태다.
#include "EncounterPatternReference.h"
#include "ValtanCombatObjectSoundCueDocument.h"
#include "ValtanPatternShakeCueDocument.h"
#include "ValtanPatternSoundCueDocument.h"
#include "ValtanPatternTree.h"
#include "ValtanCinematicEffectLibrary.h"
#include "ValtanViewAdmission.h"

// array는 lane 개수와 ImGui 입력 버퍼처럼 크기가 정해진 저장소에, vector는 순서가 있는 표시 행에 쓴다.
// optional은 대기 요청의 유무를 값과 묶고, map은 resource ID에 label 정보를 연결한다. 저장 ID는 이 container의 위치와 별개다.
#include <array>
#include <cstdint>
#include <optional>
#include <map>
#include <string>
#include <vector>

// Client 이름 공간에 편집기 계약을 모은다. 이름 공간은 이름 충돌 범위를 정하며 객체나 상태의 소유자가 아니다.
NS_BEGIN(Client)

// 아래 도구는 포인터로만 선언하므로 전방 선언이면 된다. 실제 메서드를 호출하는 CPP에서 해당 헤더를 포함한다.
class CBalanceTool;
class CValtanBossTool;
struct CAMERA_TOOL_OPEN_REQUEST;
struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST;

/* 기존 타입별 소유 도구를 하나의 편집 화면으로 연결한다. 런타임 실행 상태나 게시 Product 문서를 새로 소유하지 않는다.
   CValtanPatternTree가 읽기 모델을 제공하고 CBalanceTool이 gameplay draft를, Animation Tool이 모델 자세 미리보기를 소유한다.
   Effect/Camera 편집은 기존 소유 도구를 여는 요청으로 전달한다. 이 클래스에는 화면 선택·입력 대기·표시용 복사본을 둔다. */
class CValtanActionWorkbench final : public ICompositionWorkbenchSession
{
public:
	// DETAIL_OWNER는 선택 시 열 편집 주체, TIMELINE_LANE은 행의 표시 위치다.
	// 같은 lane에도 다른 주체의 행이 들어올 수 있어 하나의 enum으로 합치지 않는다.
	enum class DETAIL_OWNER : uint8_t
	{
		PATTERN,
		GAMEPLAY_STAGE,
		ANIMATION,
		EFFECT,
		SOUND,
		CAMERA,
		WORLD,
		COMBAT_OBJECT,
		SCENE_PROFILE,
		LIGHT,
	};

	enum class TIMELINE_LANE : uint8_t
	{
		STAGE,
		ANIMATION,
		LOGIC,
		SUMMON,
		WORLD,
		SCENE_PROFILE,
		EFFECT,
		COLLIDER,
		SOUND,
		CAMERA,
		LIGHT,
		COUNT,
	};

	using RESOURCE_DOMAIN = COMPOSITION_RESOURCE_DOMAIN;

	enum class EFFECT_RESOURCE_KIND : uint8_t
	{
		V1_PATTERN,
		V2_LEAF,
		V2_GROUP,
	};

	// timeline 한 행의 표시용 값이다. Pattern/Stage/occurrence ID를 복사해 vector 재배치와 원본 교체 후에도 대상을 다시 찾는다.
	// iStartMs/iEndMs는 선택 preview 경로 전체의 ms이고 iSubrow는 겹침을 피하기 위한 표시 행 번호다.
	// 표시 행을 수정하는 것으로 원본을 바꾸지 않고 해당 소유 도구의 typed setter에 변경을 보낸다.
	struct TIMELINE_ITEM final
	{
		DETAIL_OWNER eOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
		TIMELINE_LANE eLane = TIMELINE_LANE::STAGE;
		std::string strPatternId;
		std::string strStageId;
		std::string strStableId;
		std::string strAssetId;
		std::string strLabel;
		uint32_t iStartMs = 0u;
		uint32_t iEndMs = 0u;
		bool_t bEditable = false;
		std::size_t iSubrow = 0u;
		float fMinimumDisplayWidthPx = 4.f;
		bool_t bEffectV2Binding = false;
		bool_t bEffectV2Group = false;
		uint32_t iEffectV2BindingStartMs = 0u;
		/* 타입이 정해진 clip binding은 occurrence 하나를 정확히 가리킨다.
		   Details 편집·드래그·재생이 같은 원본 구간을 찾도록 bindingId와 clip occurrence ID를 함께 보관한다. */
		std::string strEffectV2ClipOccurrenceId;
		f32_t fEffectV2ClipPlayRate = 1.f;
		/* 반복 slot의 끝은 Server Stage의 시각에 맞춘다. 오른쪽 경계는 Stage 길이를 나타내며 다른 Stage로 옮기지 않는다. */
		bool_t bLoopsToStageEnd = false;
	};

	/* 정확한 PRIMARY 원본 Sequence에서 이를 연결한 저작 관리 Pattern을 역으로 찾는 표시용 인덱스다.
	   Server Complete Play 허용 여부는 별도로 확인한다. Owners의 포인터는 m_iCanonicalDisplayGeneration의 view를 차용하며
	   새 canonical view를 표시하기 전에 폐기한다. */
	struct SOURCE_SEQUENCE_OWNER_INDEX_ENTRY final
	{
		uint32_t iSourceActionId = 0u;
		uint32_t iSequenceIndex = 0u;
		std::vector<const VALTAN_PATTERN_VIEW*> Owners;
	};

	/* Browser의 이동 요청은 canonical frame view 안의 포인터를 보관하지 않는다.
	   모든 Composition 창의 그리기와 Save/Discard 선택이 끝날 때까지 Pattern·선택적 Stage를 stable ID로 기억한다. */
	struct PENDING_PATTERN_SELECTION final
	{
		std::string strPatternId;
		std::string strStageId;
		DETAIL_OWNER eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
		std::string strStableId;
		bool_t bOpenSequencer = false;
	};

	enum class PENDING_PATTERN_SELECTION_DECISION : uint8_t
	{
		NONE,
		SELECT,
		SAVE_AND_SELECT,
		DISCARD_AND_SELECT,
	};

public:
	CValtanActionWorkbench(
		CAnimation_Tool* pAnimationTool,
		CBalanceTool* pBalanceTool,
		CValtanBossTool* pValtanBossTool);

	bool_t Open_Valtan();
	void Set_EnvironmentResources(std::vector<std::string> profiles, std::vector<std::string> lights)
	{
		m_SceneProfileResourceIds = std::move(profiles);
		m_LightResourceIds = std::move(lights);
	}
	/* Sequencer가 Workbench 창의 표시 여부와 관계없이 같은 canonical 로드·선택·draft·timeline cache를 사용하는 진입점이다.
	   소유 도구에 미저장 draft가 있으면 선택 변경을 거절해 Workbench의 확인 창을 통한 명시적 폐기 절차를 유지한다. */
	bool_t Ensure_CanonicalLoaded(std::string& strOutStatus);
	std::vector<std::string> Get_PatternIds() const;
	bool_t Select_PatternById(
		const std::string& strPatternId,
		std::string& strOutStatus);
	bool_t Ensure_SelectedTimeline(std::string& strOutStatus);
	[[nodiscard]] bool_t Has_DisplaySnapshot() const
	{
		return 0u != m_CanonicalView.Get_PatternCount();
	}
	/* 불변 canonical 표시 복사본이 교체될 때 증가하는 식별값이다.
	   다른 view는 읽기 전용 파생 결과를 무효화할 때만 사용하며, source나 Product의 저장 revision으로 해석하지 않는다. */
	[[nodiscard]] std::uint64_t Get_CanonicalDisplayGeneration() const noexcept
	{
		return m_iCanonicalDisplayGeneration;
	}
	[[nodiscard]] bool_t Is_FullyAdmitted() const
	{
		return Can_MutateValtanView(m_eAdmission);
	}
	void On_LevelChanged();
	/* 이 Workbench가 시작한 비동기 Save 작업의 정확한 완료 결과만 진행·소비한다.
	   MainApp이 매 프레임 호출하므로 Composition 창이 숨겨져도 소유 도구 재열기·graph 갱신·runtime 게시 후속 처리를 마친다. */
	void Update_SaveState();
	void Render();
	void On_WorkbenchDeactivated() override;
	void Select_WorkbenchTarget(COMPOSITION_WORKBENCH_TARGET target) override
	{ m_bObjectWorkspace = target == COMPOSITION_WORKBENCH_TARGET::OBJECT; }
	void Begin_WorkbenchFrame() override;
	void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) override;
	void End_WorkbenchFrame() override;
	bool Undo_Edit(std::string& status);
	bool Redo_Edit(std::string& status);
	COMPOSITION_WORKBENCH_VIEW_REQUEST Consume_WorkbenchViewRequest() override;
	bool Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND command, std::string& status) override;
	bool Insert_CompositionTransfer(const COMPOSITION_TRANSFER& transfer, std::string& status) override;
	bool Can_AppendCompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
		bool asNewStage, std::string& status) const override;
	bool Append_CompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
		bool asNewStage, std::string& status) override;
	bool_t Consume_EffectToolOpenRequest(
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& OutRequest);
	bool_t Consume_EffectResourceOpenRequest(EFFECT_RESOURCE_KEY& OutKey);
	bool_t Consume_EffectResourcePreviewRequest(EFFECT_RESOURCE_KEY& OutKey);
	/* Publish 완료 시 저장된 Product 결과의 revision을 기록해 All Effects가 같은 결과를 다시 열게 한다.
	   MainApp이 창 표시와 무관하게 소비하며 Effect 소유 도구가 아직 없으면 이 요청은 대기 상태로 남는다. */
	bool_t Consume_EffectGraphRefreshRequest(
		std::string& strOutExpectedSourceRevision)
	{
		if (!m_bEffectGraphRefreshRequested)
			return false;
		strOutExpectedSourceRevision = m_strEffectGraphRefreshRevision;
		m_bEffectGraphRefreshRequested = false;
		m_strEffectGraphRefreshRevision.clear();
		return true;
	}
	bool_t Consume_CameraToolOpenRequest(
		CAMERA_TOOL_OPEN_REQUEST& OutRequest);
	bool_t Consume_AnimationToolOpenRequest();
	bool_t Consume_PreviewOwnerClaimRequest();
	void Set_CinematicPreviewStatus(const std::string& status)
	{
		m_strCinematicPreviewStatus = status;
	}
	void Set_PreviewOwnerActive(bool_t bActive)
	{
		m_bPreviewOwnerActive = bActive;
	}
	/* Sequencer에 제공하는 읽기 전용 timeline 투영이다.
	   Get_TimelineCachePatternId()에 해당하는 마지막 Build_Timeline 결과를 차용한다. */
	[[nodiscard]] const std::vector<TIMELINE_ITEM>& Get_TimelineItems() const
	{
		return m_TimelineItems;
	}
	[[nodiscard]] const std::string& Get_TimelineCachePatternId() const
	{
		return m_strTimelineCachePatternId;
	}
	[[nodiscard]] const std::string& Get_SelectedPatternId() const
	{
		return m_strSelectedPatternId;
	}
	[[nodiscard]] uint32_t Get_PlayheadMs() const
	{
		return m_iPlayheadMs;
	}
	[[nodiscard]] uint32_t Get_TimelineDurationMs() const
	{
		return m_iTimelineDurationMs;
	}
	/* Workbench 눈금자와 같은 경로로 공용 playhead와 현재 preview를 탐색한다.
	   허용된 preview를 탐색하지 못하면 false를 반환하지만 playhead 값 자체는 갱신된다. */
	bool_t Set_PlayheadMs(uint32_t iPlayheadMs);

private:
	COMPOSITION_TRANSFER Capture_CompositionSelection(std::string& status);
	bool Apply_CompositionTransfer(const COMPOSITION_TRANSFER& transfer,
		const std::string& patternId, const std::string& stageId,
		uint32_t playheadMs, bool duplicate, std::string& status);
	struct PENDING_COMPOSITION_PASTE final
	{
		COMPOSITION_TRANSFER transfer;
		std::string patternId, stageId;
		uint32_t playheadMs = 0u;
		bool duplicate = false;
	};
	std::optional<PENDING_COMPOSITION_PASTE> m_PendingCompositionPaste;
	bool m_bCompositionResourcesFocused = false;
	struct PENDING_RESOURCE_APPEND final
	{
		COMPOSITION_ANIMATION_RESOURCE Resource;
		bool bAsNewStage = false;
		std::string strPatternId;
		std::string strStageId;
	};
	bool Apply_CompositionResourceAppend(const PENDING_RESOURCE_APPEND& command,
		std::string& status);
	std::optional<PENDING_RESOURCE_APPEND> m_PendingResourceAppend;
	bool_t m_bCompositionResourceDraftReady = false;
	bool_t m_bWorkbenchBossLoadAttempted = false;
	bool_t Reload_Canonical(bool_t bReuseCommittedSource = false);
	bool_t Stage_ProductFallback(
		const CValtanCanonicalProductReadAdmission& Admission,
		const std::string& strStrictFailure);
	bool_t Save_Reload();
	void Publish_AfterSave();
	/* Save 완료 결과를 추적한다. source 저장에 성공했다면 이후 재열기에 실패해도 SOURCE_COMMITTED를 기록한다.
	   따라서 runtime-set 게시가 조용히 누락되지 않으며 Retry Publish는 source를 다시 쓰지 않고 같은 저장 revision에서 이어간다. */
	enum class POST_SAVE_STATE : uint8_t
	{
		IDLE,
		SOURCE_COMMITTED,
		APPLY_FAILED,
		PUBLISH_RUNNING,
		PUBLISH_FAILED,
		PUBLISHED_RESTART_REQUIRED,
	};
	POST_SAVE_STATE m_ePostSaveState = POST_SAVE_STATE::IDLE;
	std::string m_strPostSaveRevision;
	std::string m_strPostSaveApplyStatus;
	bool_t m_bEffectGraphRefreshRequested = false;
	std::string m_strEffectGraphRefreshRevision;
	void Mark_SourceCommitted(const std::string& strExactSourceRevision);
	void Clear_PendingSaveOwnerReceipt();
	bool_t Accept_PendingSaveOwners(std::string& strOutStatus);
	bool_t Reload_AfterPendingSave(std::string& strOutStatus);
	void Update_PostSaveState();
	void Render_PostSaveState();
	bool_t Is_PatternSoundDraftDirty(std::string& strOutStatus) const;
	bool_t Has_UnsavedCompositionDrafts(std::string& strOutStatus) const;
	bool_t Discard_CompositionDraftsAndReload();
	bool_t Validate_ManualStageTopologySoundDependencies(
		const VALTAN_PATTERN_VIEW& CandidatePattern,
		std::string& strOutStatus) const;
	void Normalize_Selection();
	void Request_PatternSelection(
		const VALTAN_PATTERN_VIEW& Pattern,
		const std::string& strStageId = {},
		DETAIL_OWNER eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE,
		const std::string& strStableId = {},
		bool_t bOpenSequencer = false);
	void Render_PendingPatternSelectionModal();
	void Resolve_PendingPatternSelection();
	void Select_Pattern(const VALTAN_PATTERN_VIEW& Pattern);
	void Select_Stage(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		DETAIL_OWNER eOwner = DETAIL_OWNER::GAMEPLAY_STAGE,
		const std::string& strStableId = {},
		bool_t bPreserveTimelineSelection = false);
	const VALTAN_PATTERN_VIEW* Find_SelectedPattern() const;
	const VALTAN_PATTERN_VIEW* Find_PatternById(
		const std::string& strPatternId) const;
	const VALTAN_STAGE_VIEW* Find_SelectedStage(
		const VALTAN_PATTERN_VIEW* pPattern) const;
	std::vector<const VALTAN_PATTERN_VIEW*> Collect_Patterns() const;
	std::vector<const VALTAN_PATTERN_VIEW*>
		Collect_CanonicalPatternsForDependencyValidation() const;
	void Invalidate_SourceSequenceOwnerIndex();
	void Ensure_SourceSequenceOwnerIndex();
	const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY* Find_SourceSequenceOwners(
		uint32_t iSourceActionId,
		uint32_t iSequenceIndex) const;
	void Invalidate_TimelineCache();
	void Invalidate_EffectivePatternCache();
	void Ensure_TimelineCache(const VALTAN_PATTERN_VIEW* pPattern);
	void Build_Timeline(const VALTAN_PATTERN_VIEW& Pattern);
	void Pack_TimelineSubrows();
	uint32_t Resolve_ClipSourceToStageMs(
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		uint32_t iSourceMs) const;

	/* true이면 canonical 재로드 또는 저장을 시도한 상태다.
	   현재 ImGui 프레임에서 이미 얻은 Pattern/Stage 포인터를 이후에 재사용하면 안 된다. */
	bool_t Render_Toolbar(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bMutationAdmitted);
	void Render_Browser(
		const VALTAN_PATTERN_VIEW* pEffectiveSelectedPattern);
	void Render_ProductFallbackBrowser();
	void Render_WorldObjectResources();
	void Render_SupplementalResources(RESOURCE_DOMAIN domain,
		const VALTAN_PATTERN_VIEW* pattern, const VALTAN_STAGE_VIEW* stage, bool editable);
	bool_t Validate_EffectV2BindingClock(
		const VALTAN_STAGE_VIEW& Stage,
		const EFFECT_V2_BINDING& Binding,
		std::string& strOutStatus) const;
	void Render_SequenceBrowser(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted);
	bool_t Reload_AnimationSequences();
	bool_t Append_SelectedSequenceToPattern(const VALTAN_PATTERN_VIEW& Pattern);
	bool_t Apply_SelectedSequenceToStage(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bAppend);
	bool_t Remove_AnimationOccurrence(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		std::string& strOutStatus);
	bool_t Duplicate_AnimationOccurrence(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		std::string& strOutStatus);
	bool_t Reorder_AnimationOccurrence(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		std::size_t iTargetIndex,
		std::string& strOutStatus);
	bool_t Transfer_AnimationOccurrence(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& SourceStage,
		const VALTAN_STAGE_VIEW& TargetStage,
		const std::string& strClipOccurrenceId,
		std::size_t iTargetIndex,
		std::string& strOutStatus);
	enum class TIMELINE_GROUP_EDIT : uint8_t { MOVE_BOXES, DUPLICATE_BOXES, DELETE_BOXES };
	struct TIMELINE_SELECTION final
	{
		std::string strPatternId, strStageId, strStableId;
		DETAIL_OWNER eOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
		bool operator==(const TIMELINE_SELECTION&) const = default;
	};
	static TIMELINE_SELECTION Timeline_SelectionKey(const TIMELINE_ITEM& Item);
	bool_t Is_TimelineBoxSelected(const TIMELINE_ITEM& Item) const;
	void Select_TimelineBox(const TIMELINE_ITEM& Item, bool_t bToggle);
	void Prune_TimelineSelection();
	bool_t Apply_TimelineGroupEdit(const VALTAN_PATTERN_VIEW& Pattern,
		TIMELINE_GROUP_EDIT edit, int64_t iWallDeltaMs, std::string& status);
	bool_t Duplicate_SelectedTimelineBox(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		std::string& strOutStatus);
	bool_t Move_SelectedStages(const VALTAN_PATTERN_VIEW& Pattern, int64_t deltaMs, std::string& status);
	bool_t Reorder_SelectedStages(const VALTAN_PATTERN_VIEW& Pattern, int32_t direction, std::string& status);
	bool_t Delete_SelectedStages(const VALTAN_PATTERN_VIEW& Pattern, std::string& strOutStatus);
	bool_t Delete_SelectedStage(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		std::string& strOutStatus, bool_t bRefreshPreview = true);
	bool_t Delete_SelectedTimelineBox(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		std::string& strOutStatus);
	void Refresh_EffectV2LocalPreviewAfterMutation(
		const VALTAN_PATTERN_VIEW* pPattern,
		std::string& strInOutStatus);
	void Refresh_PatternLocalPreviewAfterMutation(
		const VALTAN_PATTERN_VIEW* pPattern,
		std::string& strInOutStatus);
	bool_t Trim_AnimationOccurrence(const VALTAN_PATTERN_VIEW& Pattern,
        const VALTAN_STAGE_VIEW& Stage, const std::string& occurrenceId,
        bool_t startEdge, int64_t wallDeltaMs, std::string& status);
	bool_t Apply_AnimationOccurrenceTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const std::string& strClipOccurrenceId,
		uint32_t iSourceStartMs,
		uint32_t iPlayMs,
		std::string& strOutStatus);
	void Render_SelectedAnimationTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		bool_t bMutationAdmitted);
	bool_t Seek_EffectivePreview(
		const VALTAN_PATTERN_VIEW& Pattern,
		uint32_t iPositionMs,
		bool_t bPause,
		std::string& status);
	bool_t Play_EffectivePreview(
		const VALTAN_PATTERN_VIEW& Pattern,
		std::string& status);
	bool_t Play_ServerVerification(
		const VALTAN_PATTERN_VIEW& Pattern,
		std::string& status);
	void Render_AnimationStageDetails(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bMutationAdmitted);
	void Render_Preview(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bLocalPreviewAdmitted,
		bool_t bServerVerificationAdmitted);
	void Render_Details(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_GameplayStageDetails(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bMutationAdmitted);
	bool_t Apply_StageDurationDraft(const VALTAN_PATTERN_VIEW& pattern,
		const std::string& stageId, uint32_t requestedDurationMs, std::string& status);
	bool_t Apply_AuxiliaryTimelineTiming(const VALTAN_PATTERN_VIEW& pattern,
		const TIMELINE_ITEM& item, uint32_t newStartMs, uint32_t newEndMs,
		bool trim, std::string& status);
	void Render_CombatObjectSoundDetails(const std::string& archetypeId,
		const std::string& hitId, bool_t editable);
	bool_t Render_AuxiliaryDetails(const VALTAN_PATTERN_VIEW& pattern,
		const VALTAN_STAGE_VIEW& stage, bool editable);
	void Render_Timeline(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bLocalPreviewAdmitted,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_SelectedStageGapControl(
		const VALTAN_PATTERN_VIEW& Pattern,
		bool_t bPatternMutationAdmitted);
	bool_t Validate_TimelineDependencyWindows(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pEffectOverride,
		const VALTAN_PATTERN_SOUND_CUE* pSoundOverride,
		std::string& strOutStatus) const;
	bool_t Has_PendingSelectedEffectDetails(const VALTAN_PATTERN_VIEW* Pattern) const;
	bool_t Apply_EffectOccurrenceDetails(
        const VALTAN_PATTERN_VIEW& Pattern, const VALTAN_STAGE_VIEW& Stage,
        const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Current,
        const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate, std::string& status);
	bool_t Apply_EffectOccurrenceTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Current,
		uint32_t iSourceStartMs,
		uint32_t iSourceEndMs,
		std::string& strOutStatus);
	bool_t Apply_SoundTimelinePlacement(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& SourceStage,
		const VALTAN_PATTERN_SOUND_CUE& Current,
		uint32_t iPatternStartMs, uint32_t iPlaybackOffsetMs,
		std::optional<uint32_t> PlaybackDurationMs,
		std::string& strOutStatus, bool_t bRefreshPreview = true);
	bool_t Apply_PatternSoundOccurrenceTiming(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PATTERN_SOUND_CUE& Current,
		uint32_t iSourceStartMs,
		std::string& strOutStatus);
	void Render_SemanticLinkedRows(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage);
	void Request_EffectOwner(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue);
	bool_t Append_EffectResourceAtPatternEnd(const std::string& patternId, std::string& status);
	void Reload_SemanticValtanEffects(bool_t bReloadCatalog = true);
	void Reset_EffectCueEditor();
	void Render_WindowMenu();
	bool_t Render_SessionWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bPatternMutationAdmitted);
	bool m_bObjectWorkspace = false;
	void Render_PatternsWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bPatternMutationAdmitted);
	void Render_PatternsPane(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bPatternMutationAdmitted);
	void Render_PreviewWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bLocalPreviewAdmitted,
		bool_t bServerVerificationAdmitted);
	void Render_SequencerWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		bool_t bLocalPreviewAdmitted,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_DetailsWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_ResourcesWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_ResourcesPane(
		const VALTAN_PATTERN_VIEW* pPattern,
		const VALTAN_STAGE_VIEW* pStage,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_BossPatternWindow(
		const VALTAN_PATTERN_VIEW* pPattern,
		std::uint64_t iPatternViewDraftGeneration,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	void Render_BossPatternPane(
		const VALTAN_PATTERN_VIEW* pPattern,
		std::uint64_t iPatternViewDraftGeneration,
		bool_t bMutationAdmitted,
		bool_t bPatternMutationAdmitted);
	bool_t Render_BossPatternStageTopologyControls(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage,
		bool_t bPatternMutationAdmitted);
	void Request_LaneAuthoring(
		TIMELINE_LANE eLane,
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);

	const char_t* Admission_Label() const;
	static const char_t* Owner_Label(DETAIL_OWNER eOwner);
	static const char_t* Lane_Label(TIMELINE_LANE eLane);
	static uint32_t Lane_Color(TIMELINE_LANE eLane);

private:
	// 공용 shell의 각 pane은 End_WorkbenchFrame까지 같은 불변 view를 차용한다.
	const VALTAN_PATTERN_VIEW* m_pWorkbenchFramePattern = nullptr;
	const VALTAN_STAGE_VIEW* m_pWorkbenchFrameStage = nullptr;
	std::uint64_t m_iWorkbenchFrameDraftGeneration = 0u;
	bool_t m_bWorkbenchFrameLocalPreviewAdmitted = false;
	bool_t m_bWorkbenchFrameMutationAdmitted = false;
	bool_t m_bWorkbenchFramePatternMutationAdmitted = false;
	bool_t m_bWorkbenchFrameActive = false;
	bool_t m_bWorkbenchReloadRequested = false;
	bool_t m_bSharedPaneRendering = false;
	int32_t m_iWorkbenchReportedPatternTab = -1;
	bool_t m_bWorkbenchReportedTimelineMaximized = false;

	// MainApp이 unique_ptr로 가진 도구들을 차용한다. 생성자가 받은 주소를 보관하며 이 클래스가 delete하지 않는다.
	// MainApp::Free는 Workbench를 먼저 해제한 뒤 Animation/Balance/Boss Tool을 해제한다.
	// 프레임·pane 호출과 이 멤버 갱신은 MainApp의 UI 흐름에서 수행하며 worker가 이 포인터로 UI 상태를 직접 바꾸지 않는다.
	CAnimation_Tool* m_pAnimationTool = nullptr;
	CBalanceTool* m_pBalanceTool = nullptr;
	CValtanBossTool* m_pValtanBossTool = nullptr;

	// 로드에 성공한 정본의 읽기 모델을 값으로 소유한다. 원본 파일 자체나 BalanceTool의 편집 draft가 아니다.
	// 선택·프레임 포인터와 역방향 인덱스는 이 저장소 또는 effective cache를 차용하므로 교체 시 함께 무효화해야 한다.
	VALTAN_PATTERN_TREE_VIEW m_CanonicalView;
	CEncounterPatternReference m_ProductFallbackEncounter;
	/* Valtan Boss Tool의 Complete Play 허용 여부를 조회하는 Product 목록이다.
	   Browser의 canonical 목록과 역할이 다르다. Find_PatternById는 호환 Pattern도 찾지만 Server 재생은 이 목록으로 별도 판단한다. */
	VALTAN_TOOL_AUDITION_INVENTORY m_PlayableInventory;
	std::vector<SOURCE_SEQUENCE_OWNER_INDEX_ENTRY>
		m_SourceSequenceOwnerIndex;
	std::uint64_t m_iCanonicalDisplayGeneration = 0u;
	std::uint64_t m_iSourceSequenceOwnerIndexGeneration =
		~std::uint64_t{ 0u };
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT m_PatternShakes;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT m_CombatObjectSounds;
	VALTAN_VIEW_ADMISSION m_eAdmission = VALTAN_VIEW_ADMISSION::UNLOADED;
	DETAIL_OWNER m_eDetailOwner = DETAIL_OWNER::PATTERN;
	bool_t m_bLoadAttempted = false;
	bool_t m_bProductFallbackReady = false;
	/* publisher가 canonical 바이트 범위 잠금을 가진 동안 읽기 승인이 일시 실패할 수 있다.
	   이를 영구적인 빈 Browser로 굳히지 않도록 Render가 재시도 시각을 확인하고 다시 로드를 요청한다. */
	bool_t m_bCanonicalReloadRetryPending = false;
	double m_dNextCanonicalReloadRetrySeconds = 0.0;
	bool_t m_bPatternShakesReady = false;
	bool_t m_bCombatObjectSoundsReady = false;
	bool_t m_bResetLayoutRequested = false;
	bool_t m_bApplyResetLayoutThisFrame = false;
	bool_t m_bPatternsWindowVisible = true;
	/* Sequence에서 Pattern 편집으로 이어지는 한 번의 탭 이동 요청이다.
	   0은 저장 Pattern Browser, 1은 Create New Pattern을 선택한다. */
	int32_t m_iRequestedPatternTab = -1;
	bool_t m_bPreviewWindowVisible = true;
	bool_t m_bSequencerWindowVisible = true;
	bool_t m_bTimelineMaximized = false;
	bool_t m_bDetailsWindowVisible = true;
	bool_t m_bResourcesWindowVisible = true;
	bool_t m_bSessionWindowVisible = false;
	bool_t m_bBossPatternWindowVisible = false;
	bool_t m_bBossPatternFocusRequested = false;
	bool_t m_bBossPatternFitRequested = false;
	bool_t m_bLoopPreview = true;
	VALTAN_PATTERN_PREVIEW_PATH m_ePreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	VALTAN_PATTERN_PREVIEW_PATH m_eStagedPreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::END;
	std::uint64_t m_iPreviewDraftGeneration = 0u;
	bool_t m_bPreviewOwnerClaimRequested = false;
	bool_t m_bPreviewOwnerActive = false;
	std::string m_strCinematicPreviewStatus;
	/* 사용자의 Save & Publish 요청 한 번을 기억한다. source Save가 소비·해제하며
	   실제 publisher 실행과 Server revision 승인은 이후의 별도 절차다. */
	bool_t m_bPublishWithPendingSave = false;
	bool_t m_bProductSourceReady = false;
	std::string m_strProductReadiness;
	bool_t m_bOpenAnimationToolRequested = false;
	bool_t m_bEffectToolOpenRequested = false;
	bool_t m_bCameraToolOpenRequested = false;
    std::uint64_t m_iTimelineGestureDraftGeneration = 0u;
    std::uint64_t m_iTimelineGestureSoundGeneration = 0u;
    std::uint64_t m_iTimelineGestureEffectRevision = 0u;
	CBoneAnimationWorkbench m_BoneEditor;
	bool_t m_bTimelineTrimActive = false;
	bool_t m_bTimelineTrimStartEdge = false;
	std::string m_strTimelineTrimPatternId;
	std::string m_strTimelineTrimStageId;
	std::string m_strTimelineTrimStableId;
	uint32_t m_iTimelineTrimMouseStartMs = 0u;
	uint32_t m_iTimelineTrimSourceEndpointMs = 0u;
	bool_t m_bTimelineMoveActive = false;
	std::string m_strTimelineMovePatternId;
	std::string m_strTimelineMoveStageId;
	std::string m_strTimelineMoveStableId;
	uint32_t m_iTimelineMoveMouseStartMs = 0u;
	uint32_t m_iTimelineMoveSourceStartMs = 0u;
	uint32_t m_iTimelineMoveSourceEndMs = 0u;

	// Undo는 선택과 소유 도구의 값 복사본을 기록한다. shared_ptr<const>는 이력 항목들이 같은 불변 값을 공유하도록 한다.
	// 디스크 revision을 과거로 돌리는 기능은 아니며 복원할 때 현재 소유 도구의 generation을 먼저 대조한다.
	struct HISTORY_SCOPE;
	struct EDIT_HISTORY_VALUES;
	struct EDIT_HISTORY_STATE;
	using HISTORY_STATE = std::shared_ptr<const EDIT_HISTORY_STATE>;
	HISTORY_STATE Capture_EditHistory();
	void Begin_EditHistory();
	void Finish_EditHistory(bool force = false);
	bool Restore_EditHistory(const EDIT_HISTORY_STATE& state, std::string& status);
	void Render_EditHistoryButtons(const char* id);
	void Process_EditHistoryRequest();
	void Clear_EditHistory();
	CEditorUndoHistory<HISTORY_STATE> m_EditHistory{16u};
	std::shared_ptr<const EDIT_HISTORY_VALUES> m_HistoryValues;
	HISTORY_STATE m_FrameHistoryBefore;
	std::array<std::uint64_t, 5> m_HistoryGenerations{};
	int m_PendingHistoryDirection = 0;
	bool m_bKeepHistoryOnReload = false;
	bool m_bHistoryWindowFocused = false;
	std::vector<TIMELINE_SELECTION> m_TimelineSelection;
	bool_t m_bTimelineMarqueeActive = false;
	bool_t m_bTimelineMarqueeDragged = false;
	float m_fTimelineMarqueeAnchorX = 0.f, m_fTimelineMarqueeAnchorY = 0.f;
	std::string m_strTimelineMarqueePatternId;
	std::vector<TIMELINE_SELECTION> m_TimelineMarqueeBefore;
	std::vector<TIMELINE_SELECTION> m_TimelineMarqueeBase;
	TIMELINE_SELECTION m_TimelineMarqueeFocus;
	int m_iTimelineGroupMoveMs = 0;
	// 선택은 포인터나 vector index 대신 stable ID로 남긴다. 목록 정렬·행 추가·canonical 재로드 뒤에는 이 ID로 다시 조회한다.
	// 종류가 다른 occurrence는 DETAIL_OWNER와 Stage ID까지 함께 사용해 같은 문자열만으로 대상을 혼동하지 않게 한다.
	std::string m_strSelectedPatternId;
	std::string m_strSelectedStageId;
	std::string m_strSelectedStableId;
	bool_t m_bDetailFocusRequested = false;
	RESOURCE_DOMAIN m_eRequestedResourceDomain = RESOURCE_DOMAIN::ANIMATION;
	bool_t m_bResourceDomainSelectionRequested = false;
	bool_t m_bResourcesWindowExpandRequested = false;
	bool_t m_bResourcesWindowFocusRequested = false;
	std::string m_strResourceTargetPatternId;
	std::string m_strResourceTargetStageId;
	std::string m_strResourceTargetClipOccurrenceId;
	std::string m_strPinnedAuthoringSourceRevision;
	std::string m_strPinnedCanonicalSourceRevision;
	std::string m_strDisplayProvenance =
		"No canonical Product display has been admitted.";
	std::string m_strStatus =
		"Load the canonical Valtan graph to begin composition.";
	std::string m_strPatternSaveStatus;
	std::string m_strSoundStatus;
	LostArk::Shared::GameplayDataRevision
		m_LastPatternSoundAutoApplyRevision{};
	std::vector<std::string> m_PatternSoundEvents;
	std::string m_strSoundAddClipOccurrenceId;
	std::string m_strSoundAddEvent;
	uint32_t m_iSoundAddStartMs = 0u;
	VALTAN_PATTERN_SOUND_REPEAT_POLICY m_eSoundAddRepeatPolicy =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	std::string m_strShakeStatus;
	std::string m_strCombatObjectSoundStatus;
	std::array<char_t, 160u> m_PatternSearch{};
	std::array<char_t, 160u> m_ResourceSearch{};
	std::array<char_t, 160u> m_EffectSearch{};
	std::array<char_t, 160u> m_SoundSearch{};
	std::vector<CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW>
		m_AnimationSequences;
	std::vector<std::size_t> m_FilteredAnimationSequenceIndices;
	COMPOSITION_RESOURCE_TREE_NODE m_AnimationResourceTree;
	std::string m_strAnimationSequenceFilterQuery;
	bool_t m_bAnimationSequenceFilterDirty = true;
	bool_t m_bAnimationSequenceLoadAttempted = false;
	std::string m_strSelectedSequenceStableId;
	int32_t m_iSelectedSequenceSkillId = -1;
	int32_t m_iSelectedSequenceIndex = -1;
	std::string m_strSourceSequenceServerPatternId;
	int32_t m_iDamageProfileSelection = 0;
	uint32_t m_iManualStageInsertDurationMs = 1000u;
	std::string m_strAnimationSequenceStatus;
	bool_t m_bAuthoringDraftDirty = false;
	bool_t m_bPatternSoundDependencyDirty = false;
	bool_t m_bConfirmDiscardPatternSoundDraft = false;
	std::optional<PENDING_PATTERN_SELECTION> m_PendingPatternSelection;
	PENDING_PATTERN_SELECTION_DECISION m_ePendingPatternSelectionDecision =
		PENDING_PATTERN_SELECTION_DECISION::NONE;
	bool_t m_bPendingPatternSelectionModalRequested = false;
	bool_t m_bSavePatternRequested = false;
	bool_t m_bPatternSaveResultAvailable = false;
	bool_t m_bPatternSaveSucceeded = false;
	/* BalanceTool Save 작업 한 건에 대응하는 호출자 측 불변 값이다. 자식 작업은 Intermediate 아래 복사본을 소유한다.
	   여기서는 저장 완료 결과가 도착했을 때 각 Sound/Effect 소유 도구가 정확한 draft 세대를 수락하도록 당시 세대와 후보 바이트를 유지한다. */
	std::uint64_t m_iPendingSaveJobId = 0u;
	std::uint64_t m_iPendingPatternSoundDraftGeneration = 0u;
	std::uint64_t m_iPendingEffectV2DraftRevision = 0u;
	std::uint64_t m_iPendingCombatObjectSoundDraftGeneration = 0u;
	std::string m_strPendingCombatObjectSoundCandidateBytes;
	bool_t m_bPendingCombatObjectSoundOwner = false;
	std::uint64_t m_iPendingPatternShakeDraftGeneration = 0u;
	std::string m_strPendingPatternShakeCandidateBytes;
	bool_t m_bPendingPatternShakeOwner = false;
	std::string m_strPendingPatternSoundCandidateBytes;
	std::string m_strPendingEffectV2CandidateBytes;
	bool_t m_bPendingPatternSoundOwner = false;
	bool_t m_bPendingEffectV2Owner = false;
	bool_t m_bPendingSavePublishesRuntime = false;
	bool_t m_bSelectAfterPendingSave = false;

	// timeline은 한 번 결합한 표시 행을 순서대로 그리므로 vector로 보관한다. lane별 겹침 수는 고정 크기 array다.
	// Pattern·경로·각 소유 도구의 generation/revision이 cache key이며 내용이 바뀌면 Build_Timeline로 다시 계산한다.
	std::vector<TIMELINE_ITEM> m_TimelineItems;
	std::array<std::size_t, static_cast<std::size_t>(TIMELINE_LANE::COUNT)> m_TimelineLaneSubrowCounts{};
	std::string m_strTimelineCachePatternId;
	VALTAN_PATTERN_PREVIEW_PATH m_eTimelineCachePreviewPath =
		VALTAN_PATTERN_PREVIEW_PATH::END;
	std::uint64_t m_iTimelineCacheDraftGeneration = ~std::uint64_t{ 0u };
	std::uint64_t m_iTimelineCacheSoundGeneration = ~std::uint64_t{ 0u };
	std::uint64_t m_iTimelineCacheEffectV2Revision = ~std::uint64_t{ 0u };
	std::uint64_t m_iTimelineCacheEffectV1Revision = ~std::uint64_t{ 0u };
	uint32_t m_iTimelineDurationMs = 0u;
	uint32_t m_iPlayheadMs = 0u;
	f32_t m_fTimelinePixelsPerSecond = 120.f;

	/* 타입이 정해진 Pattern draft를 표시용 값으로 만들면 string/vector도 복사된다.
	   선택 Pattern·canonical revision·draft generation을 cache key로 사용해 같은 key의 프레임에서는 그 복사본을 재사용한다. */
	VALTAN_PATTERN_VIEW m_EffectivePatternCache;
	std::string m_strEffectivePatternCachePatternId;
	std::string m_strEffectivePatternCacheCanonicalRevision;
	std::uint64_t m_iEffectivePatternCacheDraftGeneration =
		~std::uint64_t{ 0u };
	bool_t m_bEffectivePatternCacheReady = false;
	// Effect 준비는 다음 프레임까지 이어질 수 있다. Pattern 포인터 대신 ID·revision·재생 위치를 복사해 대기한다.
	// 재개할 때 generation이 달라졌으면 오래된 요청을 적용하지 않는다. effectIds는 준비 상태를 다시 조회할 대상 목록이다.
	struct PENDING_LOCAL_PREVIEW final
	{
		std::string patternId;
		std::vector<std::string> effectIds;
		uint64_t draftGeneration = 0u, canonicalGeneration = 0u, v1Revision = 0u, v2Revision = 0u;
		VALTAN_PATTERN_PREVIEW_PATH path = VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
		uint32_t positionMs = 0u;
		bool pause = false;
	};
	std::optional<PENDING_LOCAL_PREVIEW> m_PendingLocalPreview;

	// graph snapshot은 현재 선택과 preview 분기 override의 파생 결과다. node/edge index는 이 snapshot 내부에서만 유효하다.
	// 시도 key도 기록하므로 잘못된 동일 입력을 매 프레임 재투영하지 않고 새 입력에서 다시 시도한다.
	ACTION_COMPOSITION_GRAPH_SNAPSHOT m_BossPatternGraphSnapshot;
	ACTION_COMPOSITION_GRAPH_ERROR m_BossPatternGraphError;
	std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE>
		m_BossPatternOutcomeOverrides;
	std::string m_strBossPatternRoutePatternId;
	std::string m_strBossPatternGraphAttemptPatternId;
	std::string m_strBossPatternGraphAttemptCanonicalRevision;
	std::uint64_t m_iBossPatternGraphAttemptDraftGeneration =
		~std::uint64_t{ 0u };
	std::uint64_t m_iBossPatternGraphAttemptRouteGeneration =
		~std::uint64_t{ 0u };
	std::uint64_t m_iBossPatternRouteGeneration = 0u;
	bool_t m_bBossPatternGraphAttempted = false;
	bool_t m_bBossPatternGraphReady = false;
	f32_t m_fBossPatternPanX = 0.f;
	f32_t m_fBossPatternPanY = 0.f;
	f32_t m_fBossPatternZoom = 1.f;

	std::string m_strEffectPatternId;
	std::string m_strEffectStageId;
	std::string m_strEffectOccurrenceId;
	std::string m_strEffectAssetId;
	struct EFFECT_RESOURCE_LABEL final
	{
		std::string displayName;
		std::string searchText;
		bool fullRestore = false;
		std::string status;
		std::vector<std::string> categories;
		std::vector<std::vector<std::string>> patternCategories;
	};
	struct EFFECT_RESOURCE_ITEM final
	{
		EFFECT_RESOURCE_KIND kind = EFFECT_RESOURCE_KIND::V1_PATTERN;
		std::string assetId;
	};
	std::vector<EFFECT_RESOURCE_ITEM> m_EffectResourceRows;
	// resource ID를 key로 label·검색 정보를 찾는다. 표시는 m_EffectResourceRows의 순서를 사용하고 map을 저장 ID 발급기로 쓰지 않는다.
	// std::less<>는 string_view 등으로 조회할 때 임시 std::string을 만들지 않는 이종 key 비교를 허용한다.
	std::map<std::string, EFFECT_RESOURCE_LABEL, std::less<>> m_EffectResourceLabels;
	VALTAN_CINEMATIC_EFFECT_LIBRARY m_CinematicEffectLibrary;
	std::string m_strCinematicEffectLibraryStatus;
	bool_t m_bCinematicEffectLibraryLoadAttempted = false;
	std::optional<EFFECT_RESOURCE_KEY> m_PendingEffectResourceOpen;
	std::optional<EFFECT_RESOURCE_KEY> m_PendingEffectResourcePreview;
	bool_t m_bShowOtherEffectOwners = false;
	std::vector<std::string> m_SemanticValtanEffectAssetIds;
	std::vector<std::size_t> m_FilteredEffectAssetIndices;
	std::vector<std::string> m_EffectV2DocumentIds;
	std::vector<int32_t> m_EffectV2DocumentTypes;
	std::vector<std::string> m_EffectV2GroupIds;
	std::vector<std::size_t> m_FilteredEffectV2DocumentIndices;
	std::vector<std::size_t> m_FilteredEffectV2GroupIndices;
	COMPOSITION_RESOURCE_TREE_NODE m_EffectResourceTree;
	std::string m_strEffectFilterQuery;
	bool_t m_bEffectFilterDirty = true;
	bool_t m_bSemanticValtanEffectLoadAttempted = false;
	std::uint64_t m_iEffectV2CatalogRevision = 0u;
	std::string m_strEffectCatalogStatus;
	EFFECT_RESOURCE_KIND m_eEffectAddResourceKind =
		EFFECT_RESOURCE_KIND::V1_PATTERN;
	uint32_t m_iEffectV2AddStartMs = 0u;
	bool_t m_bEffectAddUsesStageClock = true;
	std::string m_strEffectAddAssetId;
	std::string m_strEffectAddClipOccurrenceId;
	/* Box Details의 입력은 Apply가 검증하고 기존 V2 소유 도구에 정확한 binding을 stage할 때까지 세션 draft로 남는다. */
	EFFECT_V2_BINDING m_EffectV2BindingEditDraft;
	std::string m_strEffectV2BindingEditId;
	uint64_t m_iEffectV2BindingEditRevision = 0u;
	std::array<char_t, 160u> m_EffectV2AnchorSlot{};
	std::array<char_t, 160u> m_WorldObjectSearch{};
	std::array<char_t, 160u> m_SummonResourceSearch{};
	std::vector<std::string> m_SceneProfileResourceIds;
	std::vector<std::string> m_LightResourceIds;
	std::string m_strEffectEditIdentity;
	VALTAN_PRODUCT_EFFECT_CUE_VIEW m_EffectCueEditDraft;
	std::vector<std::size_t> m_FilteredSoundEventIndices;
	COMPOSITION_RESOURCE_TREE_NODE m_SoundResourceTree;
	std::string m_strSoundFilterQuery;
	bool_t m_bSoundFilterDirty = true;
	std::string m_strCameraCueId;
};

// Client 이름 공간 끝. 문서 데이터의 정의·게시·Server 실행은 각각 기존 소유 계층에 남는다.
NS_END
