#pragma once

#include "Client_Defines.h"
#include "CameraTool.h"
#include "EncounterPatternReference.h"
#include "Network/PacketMessages.h"
#include "ValtanCinematicCameraDocument.h"
#include "ValtanPatternFlowDocument.h"
#include "ValtanPatternTree.h"

#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class IPlayerCommandSink;
class CValtanPatternSoundSourceReadAdmission;
struct VALTAN_PATTERN_SOUND_SOURCE_RECEIPT;
struct EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST;

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

	explicit CBossTool(std::shared_ptr<IPlayerCommandSink> CommandSink);
	void Open();
	/* Opens the existing ordered Flow owner and selects its tab.  The Action
	   Composition Workbench deep-links here instead of cloning a second Flow
	   document or playback service. */
	void Open_PatternFlow();
	void Update(bool_t bToolVisible);
	void Render();
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
	bool_t Restart_SelectedPattern();
	bool_t Preview_SelectedFlowSlotIsolated();
	bool_t Start_Flow();
	bool_t Start_FlowAtSlot(const std::string& strStartSlotId);
	bool_t Restart_SavedFlow();
	bool_t Request_RevivePlayer(std::string& strOutStatus);
	bool_t Reload_FlowDocument();
	bool_t Save_FlowDocument();
	void Refresh_PresentationFreshness(bool_t bForce = false);
	void Synchronize_LiveSelection();
	void Render_BossVerificationTab();
	void Render_PatternFlowTab();
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
	void Render_SelectedPattern();
	void Render_ConnectionSummary(
		const VALTAN_PATTERN_VIEW& Pattern,
		const VALTAN_STAGE_VIEW& Stage);
	const VALTAN_PATTERN_VIEW* Find_Pattern(
		const std::string& strPatternId) const;
	const VALTAN_PATTERN_VIEW* Find_AuditionPattern(
		const std::string& strPatternId) const;
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
	CValtanCinematicCameraDocument m_CameraDocument;
	CValtanPatternFlowDocument m_FlowDocument;
	std::shared_ptr<IPlayerCommandSink> m_pCommandSink;

	std::array<char_t, 128u> m_PatternSearch{};
	std::array<char_t, 128u> m_FlowPatternSearch{};
	std::array<char_t, 128u> m_NextPatternSearch{};
	std::string m_strSelectedPatternId;
	std::string m_strSelectedStageId;
	std::string m_strSelectedFlowSlotId;
	std::string m_strSelectedFlowEdgeId;
	std::string m_strFlowLinkSourceNodeId;
	std::string m_strRepeatPatternId;
	std::string m_strStatus =
		"Select a pattern, then play it through the Server.";
	std::string m_strFlowStatus =
		"Load a saved Flow, then start it through the Server.";
	std::string m_strNextPatternStatus =
		"Choose the next pattern during live Server playback or while Valtan is idle.";
	std::string m_strCameraStatus;
	std::string m_strActionFeedback;
	std::string m_strPresentationFreshnessStatus;
	std::string m_strPresentationFreshnessRevision;
	uint32_t m_iNextReviveSequence = 1u;
	uint32_t m_iFlowLinkMaximumTraversals = 1u;
	bool_t m_bOpen = false;
	bool_t m_bFocusPending = false;
	bool_t m_bSelectPatternFlowTab = false;
	bool_t m_bGraphLoadAttempted = false;
	/* m_bGraphReady owns the preserved display snapshot. Mutation requires a
	   successful reload of the current canonical generation as a separate gate. */
	bool_t m_bGraphMutationAdmitted = false;
	bool_t m_bGraphReady = false;
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
