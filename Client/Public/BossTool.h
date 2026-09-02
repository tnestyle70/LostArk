#pragma once

#include "Client_Defines.h"
#include "BossLogicFlowView.h"
#include "CameraTool.h"
#include "EncounterPatternReference.h"
#include "Network/PacketMessages.h"
#include "ValtanCinematicCameraDocument.h"
#include "ValtanPatternFlowDocument.h"
#include "ValtanPatternTree.h"
#include "ValtanViewAdmission.h"

#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class IPlayerCommandSink;
class CBalanceTool;
class CValtanPatternSoundSourceReadAdmission;
struct VALTAN_PATTERN_SOUND_SOURCE_RECEIPT;
struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST;
struct HUD_BOSS_STATE;

/* A thin observer/controller over the existing Valtan product path.
   It never samples clips, spawns Effects, or mutates gameplay locally. */
class CBossTool final
{
public:
	struct VALTAN_ARENA_ACTIVE_STATE final
	{
		bool_t bSynchronized = false;
		bool_t bOrdinaryWallsActive = false;
		bool_t bOuterRingActive = false;
		bool_t bThreeOClockFloorActive = false;
		bool_t bNineOClockFloorActive = false;
		uint32_t iDebrisActorCount = 0u;
		uint32_t iActiveCollisionCount = 0u;
		uint32_t iActiveNavigationRegionCount = 0u;
		uint64_t iNavigationRevision = 0u;
	};
	struct SERVER_PATTERN_OPTION final
	{
		std::string strPatternId;
		std::string strDisplayName;
	};

	CBossTool(
		std::shared_ptr<IPlayerCommandSink> CommandSink,
		CBalanceTool* pBalanceTool);
	void Open();
	[[nodiscard]] bool_t Is_Open() const noexcept
	{
		return m_bOpen;
	}
	/* Opens the existing ordered Flow owner and selects its tab.  The Action
	   Composition Workbench deep-links here instead of cloning a second Flow
	   document or playback service. */
	void Open_PatternFlow();
	/* Opens the one large read-only Logic Pattern window.  It reuses this Boss
	   Tool's admitted graph, live HUD cursor and canvas state; no second parser,
	   service or gameplay runtime is constructed. */
	void Open_LogicPattern();
	bool_t Consume_LogicPatternOpenRequest();
	[[nodiscard]] bool_t Is_LogicPatternOpen() const noexcept
	{
		return m_bLogicPatternOpen;
	}
	void Update(bool_t bBossToolVisible, bool_t bLogicPatternVisible);
	void Render();
	void Render_LogicPatternWindow();
	/* Integrated Workbench route.  The caller supplies only the stable Product
	   pattern identity; Boss Tool re-resolves its current Server-audition
	   inventory before submitting through the existing typed service. */
	bool_t Play_ServerPattern(
		const std::string& strPatternId,
		std::string& strOutStatus);
	/* Action Composition Workbench restarts the exact selected occurrence
	   without reaching into Boss Tool widget state.  Server-side predecessor
	   CAS remains authoritative and a stale/completed mismatch is rejected. */
	bool_t Restart_ServerPattern(
		const std::string& strPatternId,
		std::string& strOutStatus);
	/* Reserves one exact admitted Product Pattern after the current Server
	   occurrence without resetting the Arena. */
	bool_t Queue_NextServerPattern(
		const std::string& strPatternId,
		std::string& strOutStatus);
	/* Complete Play pins the Server-active revision used by the current Arena.
	   Saving files does not create a second user-facing activation gate. */
	bool_t Can_Play_ServerPattern(std::string& strOutStatus) const;
	/* Returns the exact revision admitted by Can_Play_ServerPattern.  Deferred
	   presentation owners pin their consumer reload receipt to this value rather
	   than remembering a process-global ready boolean across Server revisions. */
	bool_t Get_ServerActivePatternRevision(
		LostArk::Shared::GameplayDataRevision& OutRevision,
		std::string& strOutStatus) const;
	/* Cheap, in-memory projection for ImGui enable/disable state only. It never
	   hashes or parses the physical Product closure. Every command must still
	   pass Get_ServerActivePatternRevision/Acquire_ServerPlaybackAdmission on
	   the click edge before it mutates Server state. */
	bool_t Observe_ServerActivePatternRevision(
		LostArk::Shared::GameplayDataRevision& OutRevision,
		std::string& strOutStatus) const;
	/* Stable owner boundary used by the Animation transport: changing the
	   independent Pattern Sound S generation or reloading consumer caches is
	   forbidden while audition/Restart/Next/Flow owns an occurrence. */
	bool_t Can_CommitPatternSoundGeneration(
		std::string& strOutStatus) const;
	/* Read-only projection of the process-wide Server verdict/lifecycle for the
	   exact Boss Tool pattern request.  A different consumer or Pattern ID is
	   never reported as this caller's Complete Play status. */
	bool_t Get_ServerPatternStatus(
		const std::string& strPatternId,
		std::string& strOutStatus,
		bool_t& bOutInFlight) const;
	bool_t Get_ServerPatternOptions(
		std::vector<SERVER_PATTERN_OPTION>& outOptions,
		std::string& strOutStatus);
	/* Re-stages the canonical joined graph and every derived audition/Next
	   inventory as one commit. Owner tools call this after an external authoring
	   transaction succeeds; a rejected reload preserves the previous graph. */
	bool_t Reload_CanonicalGraph(std::string& strOutStatus);
	bool_t Set_ServerArenaPreset(
		LostArk::Shared::VALTAN_ARENA_PRESET preset,
		std::string& strOutStatus);
	bool_t Get_ServerArenaActiveState(
		VALTAN_ARENA_ACTIVE_STATE& outState,
		std::string& strOutStatus) const;
	std::string Get_ServerArenaPresetStatus() const;
	bool_t Is_ServerArenaPresetPending() const;
	bool_t Consume_CameraToolOpenRequest(
		CAMERA_TOOL_OPEN_REQUEST& outRequest);
	bool_t Consume_EffectToolOpenRequest(
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& outRequest);

private:
	bool_t Reload_Graph();
	/* A publisher owns the canonical byte-range lock only briefly. Retry only
	   transient admission/generation races; semantic join failures remain
	   fail-closed until their source data changes. */
	void Schedule_CanonicalReloadRetry();
	/* A strict split-authoring rejection must not erase the generated Product
	   inventory on a fresh Tool launch.  This fallback has no mutation or
	   playback authority; it exists only to resolve live Product identities
	   while the strict authoring failure remains visible. */
	bool_t Fail_GraphReload(
		const std::string& strStrictFailure,
		CValtanCanonicalProductReadAdmission* pCanonicalAdmission);
	bool_t Can_MutateCanonicalGraph(std::string& strOutStatus) const;
	/* Core revision observation remains bootstrap-only so the Workbench can
	   reload a stale presentation consumer. Actual playback commands use this
	   stronger gate, which requires the primary consumer to hold the same
	   immutable revision before and after admission. */
	bool_t Acquire_ServerPlaybackAdmission(
		LostArk::Shared::GameplayDataRevision& OutRevision,
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutSoundReceipt,
		CValtanPatternSoundSourceReadAdmission& SoundAdmission,
		std::string& strOutStatus) const;
	bool_t Submit_SelectedPattern();
	/* Restarts only this Tool's exact active/completed single-Pattern
	   occurrence. The Server performs a boss-only reset, preserves the current
	   arena state, and starts that Pattern again from its first Stage. */
	bool_t Restart_SelectedPattern();
	bool_t Preview_SelectedFlowSlotIsolated();
	bool_t Start_Flow(
		const LostArk::Shared::GameplayDataRevision*
			pRequiredDefinitionRevision = nullptr);
	bool_t Start_FlowAtSlot(
		const std::string& strStartSlotId,
		const LostArk::Shared::GameplayDataRevision*
			pRequiredDefinitionRevision = nullptr);
	/* Reloads the complete saved scriptedSequence and asks the Server to reset
	   the authoritative arena before Pattern 01. This is intentionally distinct
	   from Restart_SelectedPattern and retains every saved slot/order/wait. */
	bool_t Restart_SavedFlow();
	bool_t Request_RevivePlayer(std::string& strOutStatus);
	bool_t Reload_FlowDocument();
	bool_t Save_FlowDocument();
	bool_t Retry_FlowProductPublishApply();
	void Refresh_PresentationFreshness(bool_t bForce = false);
	void Synchronize_LiveSelection();
	void Render_BossVerificationTab();
	void Render_PatternFlowTab();
	void Render_LogicFlowTab();
	void Render_LogicPatternContent();
	void Render_LogicPatternInspector(
		const VALTAN_PATTERN_VIEW& Pattern,
		const HUD_BOSS_STATE& Boss,
		bool_t bLivePattern);
	void Render_ProductFallbackPatternList();
	void Render_ProductFallbackSelectedPattern();
	void Render_ProductFallbackLogicPattern();
	void Update_LogicFlowObservation();
	bool_t Project_LogicFlowView(const VALTAN_PATTERN_VIEW& Pattern);
	void Render_FlowGraphEditor();
	void Render_FlowSlotList();
	void Render_FlowSelectedSlot();
	void Render_AddPatternPopup();
	bool_t Render_AddPatternNodePopup();
	void Render_NextPatternCard();
	void Render_NextPatternPicker();
	void Render_LiveSummary();
	void Render_ActionBar();
	void Render_PatternList();
	void Render_CurrentPatternList();
	void Render_SelectedPattern();
	void Select_Pattern(const VALTAN_PATTERN_VIEW& Pattern);
	void Normalize_CurrentFlowSelection();
	void Render_ConnectionSummary(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);
	const VALTAN_PATTERN_VIEW* Find_Pattern(
		const std::string& strPatternId) const;
	const VALTAN_PATTERN_VIEW* Find_AuditionPattern(
		const std::string& strPatternId) const;
	const ENCOUNTER_PATTERN_REFERENCE* Find_ProductFallbackPattern(
		const std::string& strPatternId) const;
	const ENCOUNTER_STAGE_REFERENCE* Find_ProductFallbackStage(
		const ENCOUNTER_PATTERN_REFERENCE& Pattern,
		const std::string& strActionId) const;
	const VALTAN_STAGE_VIEW* Find_LiveStage(
		const VALTAN_PATTERN_VIEW& Pattern) const;
	const VALTAN_STAGE_VIEW* Find_SelectedStage(
		const VALTAN_PATTERN_VIEW& Pattern) const;
	const VALTAN_INDEPENDENT_EFFECT_VIEW* Find_IndependentEffect(
		const std::string& strIndependentEffectId) const;
	const VALTAN_CINEMATIC_CAMERA_CUE* Find_CameraCue(
		const std::string& strCueId) const;
	const VALTAN_PATTERN_FLOW_SLOT* Find_SelectedFlowSlot() const;
	const VALTAN_PATTERN_FLOW_NODE* Find_SelectedFlowNode() const;
	const VALTAN_PATTERN_FLOW_EDGE* Find_SelectedFlowEdge() const;
	std::vector<std::string> Build_AdmittedPatternIds() const;
private:
	VALTAN_PATTERN_TREE_VIEW m_Graph;
	VALTAN_TOOL_AUDITION_INVENTORY m_AuditionInventory;
	std::vector<std::string> m_NextPatternIds;
	CEncounterPatternReference m_EncounterReference;
	CEncounterPatternReference m_ProductFallbackEncounterReference;
	CValtanCinematicCameraDocument m_CameraDocument;
	CValtanPatternFlowDocument m_FlowDocument;
	BOSS_LOGIC_FLOW_VIEW m_LogicFlowView;
	BOSS_LOGIC_FLOW_CANVAS_STATE m_LogicFlowCanvasState;
	BOSS_LOGIC_FLOW_SELECTION m_LogicFlowSelection;
	CBossLogicFlowObservedEdgeResolver m_LogicFlowObservedEdges;
	std::shared_ptr<IPlayerCommandSink> m_pCommandSink;
	CBalanceTool* m_pBalanceTool = nullptr;

	std::array<char_t, 128u> m_PatternSearch{};
	std::array<char_t, 128u> m_FlowPatternSearch{};
	std::array<char_t, 128u> m_NextPatternSearch{};
	std::string m_strSelectedPatternId;
	std::string m_strSelectedStageId;
	std::string m_strSelectedFlowSlotId;
	std::string m_strSelectedCurrentFlowSlotId;
	std::string m_strSelectedFlowEdgeId;
	std::string m_strFlowLinkSourceNodeId;
	std::string m_strRepeatPatternId;
	std::string m_strStatus =
		"Select a pattern, then play it through the Server.";
	std::string m_strFlowStatus =
		"Load the gameplay scriptedSequence, then start it through the Server.";
	std::string m_strNextPatternStatus =
		"Choose the next pattern during live Server playback or while Valtan is idle.";
	std::string m_strCameraStatus;
	std::string m_strActionFeedback;
	std::string m_strPresentationFreshnessStatus;
	std::string m_strPresentationFreshnessRevision;
	std::string m_strLogicFlowStatus;
	uint64_t m_iLogicFlowSourceGeneration = 0u;
	uint32_t m_iNextReviveSequence = 1u;
	uint32_t m_iFlowLinkMaximumTraversals = 1u;
	bool_t m_bOpen = false;
	bool_t m_bFocusPending = false;
	bool_t m_bLogicPatternOpen = false;
	bool_t m_bLogicPatternFocusPending = false;
	bool_t m_bLogicPatternOpenRequest = false;
	bool_t m_bSelectPatternFlowTab = false;
	bool_t m_bGraphLoadAttempted = false;
	bool_t m_bCanonicalReloadRetryPending = false;
	double m_dNextCanonicalReloadRetrySeconds = 0.0;
	/* The graph/fallback payload and its authority are separate. A failed reload
	   keeps the payload visible as STALE_PRESERVED, while every command remains
	   gated by Can_MutateValtanView. */
	VALTAN_VIEW_ADMISSION m_eGraphAdmission =
		VALTAN_VIEW_ADMISSION::UNLOADED;
	bool_t m_bGraphReady = false;
	bool_t m_bProductFallbackReady = false;
	bool_t m_bNextPatternInventoryReady = false;
	bool_t m_bFollowLive = true;
	bool_t m_bRepeat = false;
	bool_t m_bReviveFeedbackPending = false;
	bool_t m_bPresentationBaselineIntact = false;
	bool_t m_bConfirmDiscardDirtyFlow = false;
	bool_t m_bFlowGraphEditor = true;
	bool_t m_hasCameraToolOpenRequest = false;
	CAMERA_TOOL_OPEN_REQUEST m_CameraToolOpenRequest;
	bool_t m_hasEffectToolOpenRequest = false;
	std::string m_strEffectToolOpenPatternId;
	std::string m_strEffectToolOpenStageId;
	std::string m_strEffectToolOpenCueOccurrenceId;
	std::string m_strEffectToolOpenEffectAssetId;
};

NS_END
