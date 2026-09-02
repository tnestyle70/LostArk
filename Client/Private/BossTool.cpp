#include "imgui.h"

#include "BossTool.h"

#include "BalanceTool.h"
#include "CombatHUDViewModel.h"
#include "Effect_Tool.h"
#include "Level_ValtanArena.h"
#include "MainApp.h"
#include "NetworkManager.h"
#include "PlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPatternFlowService.h"
#include "ValtanPatternSoundCueDocument.h"
#include "ValtanTuningCommandService.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>
#include <system_error>
#include <utility>

namespace
{
	constexpr const char_t* BOSS_PLACEMENT_ID = "boss.valtan.center";
	constexpr const char_t* CONSUMER_ID = "Boss Tool";
	constexpr const char_t* FLOW_PREVIEW_CONSUMER_ID =
		"Boss Tool Pattern Flow Preview";
	constexpr const char_t* VALTAN_CINEMATIC_ENTRANCE_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC";
	constexpr const char_t* VALTAN_IDLE_CINEMATIC_ENTRANCE_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC_IDLE";
	constexpr double CANONICAL_RELOAD_RETRY_SECONDS = 0.25;

	bool_t Is_OptionalEntryPatternId(const std::string& PatternId)
	{
		return VALTAN_CINEMATIC_ENTRANCE_PATTERN_ID == PatternId ||
			VALTAN_IDLE_CINEMATIC_ENTRANCE_PATTERN_ID == PatternId;
	}

	std::string To_Lower(std::string Text)
	{
		std::transform(
			Text.begin(), Text.end(), Text.begin(),
			[](const unsigned char Character)
			{
				return static_cast<char_t>(std::tolower(Character));
			});
		return Text;
	}

	bool_t Contains_CaseInsensitive(
		const std::string& Text,
		const std::string& Query)
	{
		return Query.empty() ||
			To_Lower(Text).find(To_Lower(Query)) != std::string::npos;
	}

	void Append_Unique(
		std::vector<std::string>& Values,
		const std::string& Value)
	{
		if (!Value.empty() && Values.end() ==
			std::find(Values.begin(), Values.end(), Value))
		{
			Values.push_back(Value);
		}
	}

	std::string Join(const std::vector<std::string>& Values)
	{
		if (Values.empty())
			return "None";
		std::ostringstream Stream;
		for (size_t i = 0u; i < Values.size(); ++i)
		{
			if (0u != i)
				Stream << ", ";
			Stream << Values[i];
		}
		return Stream.str();
	}

	void Render_ConnectionRow(
		const char_t* pLane,
		const std::string& Value)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextDisabled("%s", pLane);
		ImGui::TableSetColumnIndex(1);
		ImGui::TextWrapped("%s", Value.c_str());
	}

	void Render_LogicCounterBadge(const bool_t bHasCounterHitBranch)
	{
		if (!bHasCounterHitBranch)
			return;
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.36f, 0.86f, 0.96f, 1.f), "COUNTER");
	}

	std::string Format_Vector3(const float3_t& Value)
	{
		char_t Buffer[128]{};
		std::snprintf(
			Buffer, sizeof(Buffer), "(%.2f, %.2f, %.2f)",
			Value.x, Value.y, Value.z);
		return Buffer;
	}

	const char_t* Camera_FrameLabel(
		const Client::VALTAN_CINEMATIC_TRACKING_MODE Mode)
	{
		switch (Mode)
		{
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::WORLD: return "WORLD";
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ:
			return "BOSS_XZ";
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::BOSS_FACING:
			return "BOSS_FACING";
		case Client::VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME:
			return "PLAYER_BOSS_FRAME";
		default: return "INVALID";
		}
	}

	bool_t Is_ExactCameraInvocation(
		const Client::VALTAN_PATTERN_VIEW& Pattern,
		const Client::VALTAN_STAGE_VIEW& Stage,
		const Client::VALTAN_CAMERA_INVOCATION_VIEW& Invocation,
		const Client::VALTAN_CINEMATIC_CAMERA_CUE& Cue)
	{
		const auto StageAt = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&Stage](const Client::VALTAN_STAGE_VIEW& Candidate)
			{
				return Candidate.strStageId == Stage.strStageId &&
					Candidate.strActionId == Stage.strActionId;
			});
		if (Pattern.Stages.end() == StageAt)
			return false;
		return Cue.strPatternId == Pattern.strPatternId &&
			Cue.strStageId == Stage.strStageId &&
			Cue.strStageActionId == Stage.strActionId &&
			Cue.iStageIndex == static_cast<uint32_t>(
				StageAt - Pattern.Stages.begin()) &&
			Cue.iDurationMs == Invocation.iDurationMs &&
			Invocation.strCameraInvocationId ==
				Invocation.strCameraCueId + ".invocation" &&
			Invocation.strTrigger == "ENTER" &&
			0u == Invocation.iStartOffsetMs &&
			Invocation.strDurationPolicy == "EXPLICIT";
	}
}

Client::CBossTool::CBossTool(
	std::shared_ptr<IPlayerCommandSink> CommandSink,
	CBalanceTool* const pBalanceTool)
	: m_pCommandSink(std::move(CommandSink)),
	  m_pBalanceTool(pBalanceTool)
{
}

void Client::CBossTool::Open()
{
	m_bOpen = true;
	m_bFocusPending = true;
	if (!m_bGraphLoadAttempted)
		(void)Reload_Graph();
}

void Client::CBossTool::Open_PatternFlow()
{
	Open();
	m_bSelectPatternFlowTab = true;
}

void Client::CBossTool::Open_LogicPattern()
{
	m_bLogicPatternOpen = true;
	m_bLogicPatternFocusPending = true;
	if (!m_bGraphLoadAttempted)
		(void)Reload_Graph();
}

bool_t Client::CBossTool::Consume_LogicPatternOpenRequest()
{
	if (!m_bLogicPatternOpenRequest)
		return false;
	m_bLogicPatternOpenRequest = false;
	return true;
}

void Client::CBossTool::Update(
	const bool_t bBossToolVisible,
	const bool_t bLogicPatternVisible)
{
	if ((!bBossToolVisible || !m_bOpen) &&
		(!bLogicPatternVisible || !m_bLogicPatternOpen))
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
		return;
	}
	if (m_bCanonicalReloadRetryPending &&
		ImGui::GetTime() >= m_dNextCanonicalReloadRetrySeconds)
	{
		(void)Reload_Graph();
	}

	Synchronize_LiveSelection();
	Update_LogicFlowObservation();
	Refresh_PresentationFreshness();
	/* Logic Pattern observes the admitted graph and replicated cursor only.
	   Keeping that standalone window open must never keep Boss Tool Repeat or
	   any other mutation lifecycle alive after the Boss Tool route is hidden. */
	if (!bBossToolVisible || !m_bOpen)
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
		return;
	}
	const bool_t bFlowOwnsPlayback =
		CValtanPatternFlowService::Get().Has_PlaybackOwnership();
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	if (m_bReviveFeedbackPending && Player.isValid && 0u != Player.iCurrentHp)
	{
		m_bReviveFeedbackPending = false;
		m_strActionFeedback.clear();
	}
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Audition =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	if (CValtanPatternAuditionService::Get().Get_NextSnapshot().Is_Live() ||
		CValtanPatternAuditionService::Get().Has_PendingNextCommand())
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
	}
	if (CONSUMER_ID == Audition.strConsumerId &&
		(VALTAN_PATTERN_AUDITION_STATE::REJECTED == Audition.eState ||
		 VALTAN_PATTERN_AUDITION_STATE::ABORTED == Audition.eState))
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
		m_strStatus = Audition.strStatus;
		return;
	}

	if (!m_bRepeat || m_strRepeatPatternId.empty() ||
		bFlowOwnsPlayback ||
		CValtanPatternAuditionService::Get().Has_PlaybackOwnership() ||
		CONSUMER_ID != Audition.strConsumerId ||
		m_strRepeatPatternId != Audition.strPatternId ||
		VALTAN_PATTERN_AUDITION_STATE::COMPLETED != Audition.eState)
	{
		return;
	}
	if (nullptr == Find_AuditionPattern(m_strRepeatPatternId))
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
		m_strStatus =
			"Repeat stopped because the Pattern left the All Effects inventory.";
		return;
	}
	if (!Player.isValid || 0u == Player.iCurrentHp)
	{
		m_strStatus =
			"Pattern complete. Revive the player to continue Repeat.";
		return;
	}

	m_strSelectedPatternId = m_strRepeatPatternId;
	(void)Submit_SelectedPattern();
}

void Client::CBossTool::Schedule_CanonicalReloadRetry()
{
	m_bCanonicalReloadRetryPending = true;
	m_dNextCanonicalReloadRetrySeconds =
		ImGui::GetTime() + CANONICAL_RELOAD_RETRY_SECONDS;
}

bool_t Client::CBossTool::Reload_Graph()
{
	m_bReviveFeedbackPending = false;
	m_strActionFeedback.clear();
	m_bGraphLoadAttempted = true;
	m_bCanonicalReloadRetryPending = false;
	/* A reload attempt revokes mutation admission immediately. The last good
	   graph remains available for diagnosis, but it cannot authorize commands
	   unless this exact staging transaction commits. */
	m_eGraphAdmission = Can_DisplayValtanView(m_eGraphAdmission) ?
		VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
		VALTAN_VIEW_ADMISSION::UNLOADED;
	VALTAN_PATTERN_TREE_VIEW StagedGraph;
	VALTAN_CANONICAL_READ_DIAGNOSTIC Diagnostic;
	CValtanCanonicalProductReadAdmission CanonicalAdmission;
	if (!CanonicalAdmission.Acquire(Diagnostic))
	{
		if (Diagnostic.Is_AutomaticRetryable())
			Schedule_CanonicalReloadRetry();
		return Fail_GraphReload(Diagnostic.strStatus, nullptr);
	}
	if (!CValtanPatternTree::Load_WhileAdmitted(
			CanonicalAdmission, StagedGraph, Diagnostic))
	{
		if (Diagnostic.Is_AutomaticRetryable())
			Schedule_CanonicalReloadRetry();
		std::string Status = Diagnostic.strStatus;
		if (Diagnostic.Requires_ProductProjection())
		{
			Status = "REPROJECTION_REQUIRED: Save/Project the joined Valtan source "
				"generation before command admission. Recovery command: " +
				std::string{
					VALTAN_CANONICAL_READ_DIAGNOSTIC::PRODUCT_PROJECTION_COMMAND } +
				" | " + Status;
			if (!Diagnostic.strRejectedPatternId.empty())
			{
				Status += " | quarantined source owner=" +
					Diagnostic.strRejectedPatternId;
				if (!Diagnostic.strRejectedStageId.empty())
					Status += "/" + Diagnostic.strRejectedStageId;
			}
		}
		return Fail_GraphReload(Status, &CanonicalAdmission);
	}
	VALTAN_TOOL_AUDITION_INVENTORY StagedAuditionInventory;
	std::string InventoryError;
	if (!CValtanPatternTree::Build_PlayablePatternInventory(
			StagedGraph, StagedAuditionInventory, InventoryError))
	{
		return Fail_GraphReload(InventoryError, &CanonicalAdmission);
	}

	std::vector<std::string> StagedNextPatternIds;
	if (!CValtanPatternTree::Build_NextPatternInventory(
			StagedGraph, StagedNextPatternIds, InventoryError))
	{
		return Fail_GraphReload(InventoryError, &CanonicalAdmission);
	}
	std::vector<std::string> StagedAdmittedPatternIds;
	StagedAdmittedPatternIds.reserve(StagedAuditionInventory.Get_PatternCount());
	for (const auto* pPatternIds : {
			&StagedAuditionInventory.CorePatternIds,
			&StagedAuditionInventory.AnimatorPatternIds,
			&StagedAuditionInventory.DerivedPatternIds })
	{
		StagedAdmittedPatternIds.insert(
			StagedAdmittedPatternIds.end(),
			pPatternIds->begin(), pPatternIds->end());
	}
	const bool_t bFlowWasReady = m_FlowDocument.Is_Ready();
	CValtanPatternFlowDocument CanonicalFlowDocument;
	std::string StagedFlowStatus;
	if (!CanonicalFlowDocument.Load_CanonicalSequence(
			StagedGraph.strScriptedSequenceId,
			StagedGraph.strScriptedSequenceMode,
			StagedGraph.iScriptedSequenceInterStepPursuitMs,
			StagedGraph.ScriptedSequencePatternIds,
			StagedAdmittedPatternIds, StagedFlowStatus))
	{
		m_strFlowStatus =
			"Canonical scriptedSequence load failed; graph reload was not committed: " +
			StagedFlowStatus;
		return Fail_GraphReload(
			"gameplay scriptedSequence could not be staged: " +
			StagedFlowStatus, &CanonicalAdmission);
	}
	CValtanPatternFlowDocument StagedFlowDocument = CanonicalFlowDocument;
	if (bFlowWasReady && m_FlowDocument.Is_Dirty())
	{
		if (m_FlowDocument.Has_ExternalConflict() ||
			m_FlowDocument.Get_SourceRevision() !=
				CanonicalFlowDocument.Get_SourceRevision() ||
			!CValtanPatternFlowDocument::Validate(
				m_FlowDocument.Get_Draft(), StagedAdmittedPatternIds,
				StagedFlowStatus))
		{
			m_strFlowStatus =
				"Save, discard, or reload the canonical scriptedSequence draft: " +
				StagedFlowStatus;
			return Fail_GraphReload(
				"an unsaved sequence draft conflicts with the current gameplay revision: " +
				StagedFlowStatus, &CanonicalAdmission);
		}
		StagedFlowDocument = m_FlowDocument;
	}

	CEncounterPatternReference StagedEncounter;
	std::string EncounterStatus;
	const bool_t bEncounterReady = StagedEncounter.Load(
		CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanEncounter.json"),
		EncounterStatus);
	CValtanCinematicCameraDocument StagedCamera;
	std::string CameraStatus;
	const bool_t bCameraReady = bEncounterReady && StagedCamera.Load(
		CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanCinematicCamera.json"),
		StagedEncounter,
		CameraStatus);
	/* The sequence now lives inside the canonical gameplay source covered by
	   this same shared admission.  One final generation check protects the
	   Pattern tree, Flow projection and verification views as one snapshot. */
	std::string FinalAdmissionStatus;
	if (!CanonicalAdmission.Validate_StillCurrent(FinalAdmissionStatus))
	{
		Schedule_CanonicalReloadRetry();
		return Fail_GraphReload(
			"canonical Product generation changed before commit: " +
			FinalAdmissionStatus, nullptr);
	}

	m_Graph = std::move(StagedGraph);
	m_iLogicFlowSourceGeneration =
		(std::numeric_limits<uint64_t>::max)() ==
			m_iLogicFlowSourceGeneration ?
			1u : m_iLogicFlowSourceGeneration + 1u;
	m_LogicFlowSelection.Clear();
	m_LogicFlowObservedEdges.Reset();
	m_AuditionInventory = std::move(StagedAuditionInventory);
	m_NextPatternIds = std::move(StagedNextPatternIds);
	m_FlowDocument = std::move(StagedFlowDocument);
	m_bNextPatternInventoryReady = true;
	m_EncounterReference = bEncounterReady ?
		std::move(StagedEncounter) : CEncounterPatternReference{};
	m_CameraDocument = bCameraReady ?
		std::move(StagedCamera) : CValtanCinematicCameraDocument{};
	m_strCameraStatus = bCameraReady ?
		"Camera cues loaded." :
		"Camera lane unavailable: " +
			(bEncounterReady ? CameraStatus : EncounterStatus);
	m_bGraphReady = true;
	m_eGraphAdmission = VALTAN_VIEW_ADMISSION::ADMITTED;
	m_ProductFallbackEncounterReference.Clear();
	m_bProductFallbackReady = false;
	Refresh_PresentationFreshness(true);
	if (!bFlowWasReady)
	{
		m_strFlowStatus = StagedFlowStatus;
		const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
			m_FlowDocument.Get_DefaultFlow();
		if (nullptr != pFlow && !pFlow->Nodes.empty())
			m_strSelectedFlowSlotId = pFlow->strEntryNodeId;
	}
	const VALTAN_PATTERN_FLOW_DEFINITION* const pCommittedFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr != pCommittedFlow &&
		pCommittedFlow->Nodes.end() == std::find_if(
			pCommittedFlow->Nodes.begin(), pCommittedFlow->Nodes.end(),
			[this](const VALTAN_PATTERN_FLOW_NODE& node)
			{ return node.strNodeId == m_strSelectedFlowSlotId; }))
	{
		m_strSelectedFlowSlotId = pCommittedFlow->strEntryNodeId;
	}
	if (nullptr != pCommittedFlow &&
		pCommittedFlow->Edges.end() == std::find_if(
			pCommittedFlow->Edges.begin(), pCommittedFlow->Edges.end(),
			[this](const VALTAN_PATTERN_FLOW_EDGE& edge)
			{ return edge.strEdgeId == m_strSelectedFlowEdgeId; }))
	{
		m_strSelectedFlowEdgeId.clear();
	}
	Normalize_CurrentFlowSelection();
	m_strFlowLinkSourceNodeId.clear();

	const VALTAN_PATTERN_VIEW* pSelected =
		Find_AuditionPattern(m_strSelectedPatternId);
	if (nullptr == pSelected)
	{
		m_strSelectedPatternId.clear();
		m_strSelectedStageId.clear();
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
	}
	if (nullptr != pSelected)
	{
		const auto Stage = std::find_if(
			pSelected->Stages.begin(), pSelected->Stages.end(),
			[this](const VALTAN_STAGE_VIEW& Candidate)
			{
				return Candidate.strStageId == m_strSelectedStageId;
			});
		if (Stage == pSelected->Stages.end())
		{
			m_strSelectedStageId = pSelected->Stages.empty() ?
				std::string{} : pSelected->Stages.front().strStageId;
		}
	}
	if (!m_strRepeatPatternId.empty() &&
		nullptr == Find_AuditionPattern(m_strRepeatPatternId))
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
	}

	m_strStatus = "Canonical graph ADMITTED with " +
		std::to_string(m_AuditionInventory.Get_PatternCount()) +
		" split-owned playable patterns.";
	return true;
}

bool_t Client::CBossTool::Can_MutateCanonicalGraph(
	std::string& strOutStatus) const
{
	if (Can_MutateValtanView(m_eGraphAdmission))
	{
		strOutStatus.clear();
		return true;
	}
	strOutStatus = Can_DisplayValtanView(m_eGraphAdmission) &&
		m_bGraphReady ?
		"Canonical graph is STALE_PRESERVED. Previous rows are display-only until a fresh reload is ADMITTED." :
		(m_bProductFallbackReady ?
			"READ-ONLY PRODUCT FALLBACK has no command authority. Repair the strict split authoring join and reload it before using Save, Restart, Play, Repeat, or Next." :
			"Canonical graph is not ADMITTED; reload it before using a mutation command.");
	return false;
}

void Client::CBossTool::Refresh_PresentationFreshness(const bool_t bForce)
{
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!CNetworkManager::Get().Is_Connected() || !Boss.isValid ||
		!Boss.PinnedDefinitionRevision.Is_Valid())
	{
		m_strPresentationFreshnessRevision.clear();
		m_bPresentationBaselineIntact = false;
		m_strPresentationFreshnessStatus =
			"No Server-pinned presentation revision is active.";
		return;
	}
	const std::string Revision =
		LostArk::Shared::Format_GameplayDataRevision(
			Boss.PinnedDefinitionRevision);
	if (!bForce && Revision == m_strPresentationFreshnessRevision)
		return;
	m_strPresentationFreshnessRevision = Revision;
	m_bPresentationBaselineIntact =
		CNetworkManager::Get().Is_PresentationRevisionAvailable(
			Boss.PinnedDefinitionRevision);
	m_strPresentationFreshnessStatus = m_bPresentationBaselineIntact ?
		"Server presentation is ready." :
		"Re-enter Valtan to load the active Server revision.";
}

bool_t Client::CBossTool::Submit_SelectedPattern()
{
	m_bReviveFeedbackPending = false;
	m_strActionFeedback.clear();
	if (nullptr == Find_AuditionPattern(m_strSelectedPatternId))
	{
		m_strStatus = "Select a valid Valtan pattern first.";
		return false;
	}

	std::string Status;
	LostArk::Shared::GameplayDataRevision expectedActiveRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
	CValtanPatternSoundSourceReadAdmission SoundAdmission;
	if (!Acquire_ServerPlaybackAdmission(
			expectedActiveRevision, PinnedSoundReceipt,
			SoundAdmission, Status))
	{
		m_strStatus = std::move(Status);
		if (m_bRepeat)
		{
			m_bRepeat = false;
			m_strRepeatPatternId.clear();
		}
		return false;
	}
	if (!CValtanPatternAuditionService::Get().Submit(
			CONSUMER_ID,
			BOSS_PLACEMENT_ID,
			m_strSelectedPatternId,
			expectedActiveRevision,
			PinnedSoundReceipt,
			Status))
	{
		m_strStatus = Status;
		if (m_bRepeat)
		{
			m_bRepeat = false;
			m_strRepeatPatternId.clear();
		}
		return false;
	}

	m_strRepeatPatternId = m_bRepeat ?
		m_strSelectedPatternId : std::string{};
	m_bFollowLive = true;
	m_strStatus = Status;
	return true;
}

bool_t Client::CBossTool::Restart_SelectedPattern()
{
	m_bReviveFeedbackPending = false;
	m_strActionFeedback.clear();
	LostArk::Shared::GameplayDataRevision ReplacementRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
	std::string RevisionStatus;
	CValtanPatternSoundSourceReadAdmission SoundAdmission;
	if (!Acquire_ServerPlaybackAdmission(
			ReplacementRevision, PinnedSoundReceipt,
			SoundAdmission, RevisionStatus))
	{
		m_strStatus = std::move(RevisionStatus);
		return false;
	}
	if (nullptr == Find_AuditionPattern(m_strSelectedPatternId))
	{
		m_strStatus = "Select the active Valtan pattern before restarting it.";
		return false;
	}

	std::string Status;
	if (!CValtanPatternAuditionService::Get().Restart_ActivePattern(
			CONSUMER_ID,
			BOSS_PLACEMENT_ID,
			m_strSelectedPatternId,
			ReplacementRevision,
			PinnedSoundReceipt,
			Status))
	{
		m_strStatus = std::move(Status);
		return false;
	}
	m_bFollowLive = true;
	m_strStatus = std::move(Status);
	return true;
}

bool_t Client::CBossTool::Restart_ServerPattern(
	const std::string& strPatternId,
	std::string& strOutStatus)
{
	if (strPatternId.empty())
	{
		strOutStatus = "Select one stable Valtan Pattern before Restart.";
		return false;
	}
	LostArk::Shared::GameplayDataRevision ReplacementRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
	std::string RevisionStatus;
	CValtanPatternSoundSourceReadAdmission SoundAdmission;
	if (!Acquire_ServerPlaybackAdmission(
			ReplacementRevision, PinnedSoundReceipt,
			SoundAdmission, RevisionStatus))
	{
		strOutStatus = std::move(RevisionStatus);
		return false;
	}
	if (nullptr == Find_AuditionPattern(strPatternId))
	{
		strOutStatus =
			"Restart rejected because the selected Pattern is absent from the admitted canonical inventory.";
		return false;
	}
	m_strSelectedPatternId = strPatternId;
	m_bReviveFeedbackPending = false;
	m_strActionFeedback.clear();
	if (!CValtanPatternAuditionService::Get().Restart_ActivePattern(
			CONSUMER_ID, BOSS_PLACEMENT_ID, strPatternId,
			ReplacementRevision, PinnedSoundReceipt, strOutStatus))
	{
		m_strStatus = strOutStatus;
		return false;
	}
	m_bFollowLive = true;
	m_strStatus = strOutStatus;
	return true;
}

bool_t Client::CBossTool::Queue_NextServerPattern(
	const std::string& strPatternId,
	std::string& strOutStatus)
{
	if (strPatternId.empty())
	{
		strOutStatus = "Next Pattern requires one stable Pattern ID.";
		return false;
	}
	if ((!m_bGraphLoadAttempted || !m_bGraphReady) && !Reload_Graph())
	{
		strOutStatus = m_strStatus.empty() ?
			"Next Pattern could not admit the current canonical graph." :
			m_strStatus;
		return false;
	}
	LostArk::Shared::GameplayDataRevision ExpectedRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
	std::string RevisionStatus;
	CValtanPatternSoundSourceReadAdmission SoundAdmission;
	if (!Acquire_ServerPlaybackAdmission(
			ExpectedRevision, PinnedSoundReceipt,
			SoundAdmission, RevisionStatus))
	{
		strOutStatus = std::move(RevisionStatus);
		return false;
	}
	const VALTAN_PATTERN_VIEW* const pPattern =
		Find_AuditionPattern(strPatternId);
	if (!m_bNextPatternInventoryReady || nullptr == pPattern ||
		!pPattern->bAuthoringMasterManaged ||
		m_NextPatternIds.end() == std::find(
			m_NextPatternIds.begin(), m_NextPatternIds.end(), strPatternId))
	{
		strOutStatus =
			"Next Pattern is absent from the current admitted pending inventory: " +
			strPatternId + ".";
		return false;
	}
	CValtanPatternAuditionService& Service =
		CValtanPatternAuditionService::Get();
	std::string QueueStatus;
	if (!Service.Can_QueueNextPattern(
			BOSS_PLACEMENT_ID, ExpectedRevision, QueueStatus))
	{
		strOutStatus = std::move(QueueStatus);
		return false;
	}

	/* An explicit Next reservation owns the continuation decision. */
	m_bRepeat = false;
	m_strRepeatPatternId.clear();
	const bool_t bQueued = Service.Queue_NextPattern(
		CONSUMER_ID, BOSS_PLACEMENT_ID, strPatternId,
		ExpectedRevision, PinnedSoundReceipt, QueueStatus);
	m_strNextPatternStatus = QueueStatus;
	strOutStatus = std::move(QueueStatus);
	return bQueued;
}

bool_t Client::CBossTool::Can_Play_ServerPattern(
	std::string& strOutStatus) const
{
	LostArk::Shared::GameplayDataRevision Revision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT SoundReceipt;
	CValtanPatternSoundSourceReadAdmission SoundAdmission;
	return Acquire_ServerPlaybackAdmission(
		Revision, SoundReceipt, SoundAdmission, strOutStatus);
}

bool_t Client::CBossTool::Get_ServerActivePatternRevision(
	LostArk::Shared::GameplayDataRevision& OutRevision,
	std::string& strOutStatus) const
{
	OutRevision = {};
	if (!Can_MutateCanonicalGraph(strOutStatus))
		return false;
	CValtanTuningCommandService& TuningService =
		CValtanTuningCommandService::Get();
	TuningService.Update();
	const VALTAN_TUNING_COMMAND_SNAPSHOT& TuningSnapshot =
		TuningService.Get_Snapshot();
	if (TuningService.Has_PendingCommand())
	{
		strOutStatus =
			"Server Pattern playback is blocked while the latest saved Product candidate has no terminal Server verdict.";
		if (!TuningSnapshot.strStatus.empty())
			strOutStatus += " " + TuningSnapshot.strStatus;
		return false;
	}
	const LostArk::Shared::GameplayDataRevision activeRevision =
		CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
	if (!CNetworkManager::Get().Is_Connected() || !activeRevision.Is_Valid())
	{
		strOutStatus =
			"Complete Play requires one exact Server-active definition revision.";
		return false;
	}
	if (TuningService.Has_GameplaySourceActivationExpectation())
	{
		LostArk::Shared::GameplayDataRevision SavedCandidateRevision{};
		if (!TuningService.Try_GetLatestGameplaySourceServerActiveRevision(
				SavedCandidateRevision, strOutStatus))
		{
			if (!TuningSnapshot.strCandidateRevision.empty() &&
				!TuningSnapshot.bCandidateIsServerActive)
			{
				strOutStatus +=
					" The published candidate is not the Server-active revision.";
			}
			return false;
		}
		if (SavedCandidateRevision != activeRevision)
		{
			strOutStatus =
				"The latest saved Product candidate changed while resolving the Server-active revision.";
			return false;
		}
		OutRevision = SavedCandidateRevision;
	}
	else
	{
		OutRevision = activeRevision;
	}
	strOutStatus.clear();
	return true;
}

bool_t Client::CBossTool::Observe_ServerActivePatternRevision(
	LostArk::Shared::GameplayDataRevision& OutRevision,
	std::string& strOutStatus) const
{
	OutRevision = {};
	if (!Can_MutateCanonicalGraph(strOutStatus))
		return false;
	const CNetworkManager& Network = CNetworkManager::Get();
	const LostArk::Shared::GameplayDataRevision ActiveRevision =
		Network.Get_GameplayRevisionState().ServerActiveRevision;
	if (!Network.Is_Connected() || !ActiveRevision.Is_Valid())
	{
		strOutStatus =
			"Server Pattern playback requires one active Server revision.";
		return false;
	}
	OutRevision = ActiveRevision;
	strOutStatus =
		"Runtime revision observed. Exact Product/Sound admission runs when the command is pressed.";
	return true;
}

bool_t Client::CBossTool::Can_CommitPatternSoundGeneration(
	std::string& strOutStatus) const
{
	CValtanPatternAuditionService& Audition =
		CValtanPatternAuditionService::Get();
	CValtanPatternFlowService& Flow = CValtanPatternFlowService::Get();
	Audition.Update();
	Flow.Update();
	if (Audition.Has_PatternSoundMutationBarrier())
	{
		strOutStatus =
			"Pattern Sound changes are blocked while Complete Play, Restart or Next owns an active request.";
		return false;
	}
	if (Flow.Has_PatternSoundMutationBarrier())
	{
		strOutStatus =
			"Pattern Sound changes are blocked while Pattern Flow is active.";
		return false;
	}
	strOutStatus.clear();
	return true;
}

bool_t Client::CBossTool::Acquire_ServerPlaybackAdmission(
	LostArk::Shared::GameplayDataRevision& OutRevision,
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutSoundReceipt,
	CValtanPatternSoundSourceReadAdmission& SoundAdmission,
	std::string& strOutStatus) const
{
	OutRevision = {};
	OutSoundReceipt = {};
	LostArk::Shared::GameplayDataRevision PreAdmissionRevision{};
	if (!Get_ServerActivePatternRevision(
			PreAdmissionRevision, strOutStatus))
	{
		return false;
	}
	const CLevel_ValtanArena* const pArena =
		CLevel_ValtanArena::Get_Active();
	if (nullptr == pArena)
	{
		strOutStatus =
			"Server Pattern playback requires the active Valtan Arena and its primary replicated presentation consumer.";
		return false;
	}
	if (!pArena->Can_Play_PrimaryValtanPresentation(
			PreAdmissionRevision, strOutStatus))
	{
		return false;
	}
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT ConsumerSoundReceipt;
	if (!pArena->Get_PrimaryValtanPatternSoundSourceReceipt(
			ConsumerSoundReceipt, strOutStatus))
	{
		return false;
	}
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT CurrentSoundReceipt;
	if (!SoundAdmission.Acquire(CurrentSoundReceipt, strOutStatus))
		return false;
	if (CurrentSoundReceipt != ConsumerSoundReceipt)
	{
		strOutStatus =
			"Server Pattern playback needs the latest saved Sound. Reload Sound, then try again.";
		return false;
	}
	LostArk::Shared::GameplayDataRevision PostAdmissionRevision{};
	if (!Get_ServerActivePatternRevision(
			PostAdmissionRevision, strOutStatus) ||
		PostAdmissionRevision != PreAdmissionRevision)
	{
		strOutStatus =
			"The Server-active Pattern revision changed across presentation admission; no command was submitted.";
		return false;
	}
	OutRevision = PreAdmissionRevision;
	OutSoundReceipt = CurrentSoundReceipt;
	strOutStatus.clear();
	return true;
}

bool_t Client::CBossTool::Get_ServerPatternOptions(
	std::vector<SERVER_PATTERN_OPTION>& outOptions,
	std::string& strOutStatus)
{
	outOptions.clear();
	if ((!m_bGraphLoadAttempted || !m_bGraphReady) && !Reload_Graph())
	{
		strOutStatus = m_strStatus.empty() ?
			"Boss Tool could not admit the current Valtan graph." : m_strStatus;
		return false;
	}
	const auto append = [this, &outOptions](
		const std::vector<std::string>& patternIds)
	{
		for (const std::string& patternId : patternIds)
		{
			const VALTAN_PATTERN_VIEW* const pattern = Find_Pattern(patternId);
			if (nullptr == pattern)
				continue;
			SERVER_PATTERN_OPTION option;
			option.strPatternId = pattern->strPatternId;
			option.strDisplayName = pattern->strDisplayName;
			outOptions.push_back(std::move(option));
		}
	};
	append(m_AuditionInventory.CorePatternIds);
	append(m_AuditionInventory.AnimatorPatternIds);
	append(m_AuditionInventory.DerivedPatternIds);
	if (outOptions.empty())
	{
		strOutStatus = "The current Valtan graph admitted no Complete Play pattern.";
		return false;
	}
	strOutStatus = Can_MutateValtanView(m_eGraphAdmission) ?
		"Loaded " + std::to_string(outOptions.size()) +
			" Server-admitted Complete Play patterns." :
		"Showing " + std::to_string(outOptions.size()) +
			" preserved patterns as display-only rows; canonical mutation admission is stale.";
	return true;
}

bool_t Client::CBossTool::Reload_CanonicalGraph(
	std::string& strOutStatus)
{
	const bool_t bReloaded = Reload_Graph();
	strOutStatus = m_strStatus.empty() ?
		(bReloaded ? "Boss canonical graph and audition inventory reloaded." :
			"Boss canonical graph reload was rejected.") :
		m_strStatus;
	return bReloaded;
}

bool_t Client::CBossTool::Play_ServerPattern(
	const std::string& strPatternId,
	std::string& strOutStatus)
{
	if (strPatternId.empty())
	{
		strOutStatus = "Complete Play requires a stable pattern ID.";
		return false;
	}
	if ((!m_bGraphLoadAttempted || !m_bGraphReady) && !Reload_Graph())
	{
		strOutStatus = m_strStatus.empty() ?
			"Boss Tool could not admit the current Valtan graph." : m_strStatus;
		return false;
	}
	if (nullptr == Find_AuditionPattern(strPatternId))
	{
		strOutStatus = "Pattern is not in the current Server audition inventory: " +
			strPatternId + ".";
		return false;
	}
	m_strSelectedPatternId = strPatternId;
	const bool_t submitted = Submit_SelectedPattern();
	strOutStatus = m_strStatus;
	return submitted;
}

bool_t Client::CBossTool::Get_ServerPatternStatus(
	const std::string& strPatternId,
	std::string& strOutStatus,
	bool_t& bOutInFlight) const
{
	bOutInFlight = false;
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& snapshot =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	if (CONSUMER_ID != snapshot.strConsumerId ||
		strPatternId != snapshot.strPatternId || snapshot.strStatus.empty())
	{
		return false;
	}
	strOutStatus = snapshot.strStatus;
	bOutInFlight = snapshot.Is_InFlight();
	return true;
}

bool_t Client::CBossTool::Set_ServerArenaPreset(
	const LostArk::Shared::VALTAN_ARENA_PRESET preset,
	std::string& strOutStatus)
{
#ifdef _DEBUG
	if (!Can_MutateCanonicalGraph(strOutStatus))
		return false;
	CLevel_ValtanArena* const arena = CLevel_ValtanArena::Get_Active();
	if (nullptr == arena)
	{
		strOutStatus =
			"Arena Preset requires the Server-approved Valtan Arena level.";
		return false;
	}
	return arena->Set_ArenaPreset(preset, strOutStatus);
#else
	(void)preset;
	strOutStatus = "Arena presets are available only in Debug Developer Tools.";
	return false;
#endif
}

bool_t Client::CBossTool::Get_ServerArenaActiveState(
	VALTAN_ARENA_ACTIVE_STATE& outState,
	std::string& strOutStatus) const
{
	outState = {};
#ifdef _DEBUG
	const CLevel_ValtanArena* const arena = CLevel_ValtanArena::Get_Active();
	if (nullptr == arena)
	{
		strOutStatus =
			"Arena Active requires the Server-approved Valtan Arena level.";
		return false;
	}
	const CLevel_ValtanArena::ARENA_ACTIVE_STATE source =
		arena->Get_ArenaActiveState();
	outState.bSynchronized = source.bSynchronized;
	outState.bOrdinaryWallsActive = source.bOrdinaryWallsActive;
	outState.bOuterRingActive = source.bOuterRingActive;
	outState.bThreeOClockFloorActive =
		source.bThreeOClockFloorActive;
	outState.bNineOClockFloorActive = source.bNineOClockFloorActive;
	outState.iDebrisActorCount = source.iDebrisActorCount;
	outState.iActiveCollisionCount = source.iActiveCollisionCount;
	outState.iActiveNavigationRegionCount =
		source.iActiveNavigationRegionCount;
	outState.iNavigationRevision = source.iNavigationRevision;
	strOutStatus = source.bSynchronized ?
		"Server destruction, collision and navigation state synchronized." :
		"Waiting for the Server destruction full-sync.";
	return source.bSynchronized;
#else
	strOutStatus = "Arena Active is available only in Debug Developer Tools.";
	return false;
#endif
}

std::string Client::CBossTool::Get_ServerArenaPresetStatus() const
{
#ifdef _DEBUG
	const CLevel_ValtanArena* const arena = CLevel_ValtanArena::Get_Active();
	return nullptr == arena ?
		std::string("Enter Valtan Arena to stage a Server environment preset.") :
		arena->Get_ArenaAuditionStatus();
#else
	return "Arena presets are available only in Debug Developer Tools.";
#endif
}

bool_t Client::CBossTool::Is_ServerArenaPresetPending() const
{
#ifdef _DEBUG
	const CLevel_ValtanArena* const arena = CLevel_ValtanArena::Get_Active();
	return nullptr != arena && arena->Is_ArenaPresetRequestPending();
#else
	return false;
#endif
}

bool_t Client::CBossTool::Preview_SelectedFlowSlotIsolated()
{
	const VALTAN_PATTERN_FLOW_NODE* pNode = Find_SelectedFlowNode();
	if (nullptr == pNode || nullptr == Find_AuditionPattern(pNode->strPatternId))
	{
		m_strFlowStatus = "Select a valid gameplay sequence node first.";
		return false;
	}
	if (CValtanPatternFlowService::Get().Has_PlaybackOwnership())
	{
		m_strFlowStatus =
			"Isolated preview is unavailable while Server Flow playback or a Start request is active.";
		return false;
	}
	LostArk::Shared::GameplayDataRevision ExpectedRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
	std::string RevisionStatus;
	CValtanPatternSoundSourceReadAdmission SoundAdmission;
	if (!Acquire_ServerPlaybackAdmission(
			ExpectedRevision, PinnedSoundReceipt,
			SoundAdmission, RevisionStatus))
	{
		m_strFlowStatus = std::move(RevisionStatus);
		return false;
	}

	m_bRepeat = false;
	m_strRepeatPatternId.clear();
	std::string Status;
	if (!CValtanPatternAuditionService::Get().Submit(
			FLOW_PREVIEW_CONSUMER_ID,
			BOSS_PLACEMENT_ID,
			pNode->strPatternId,
			ExpectedRevision,
			PinnedSoundReceipt,
			Status))
	{
		m_strFlowStatus = Status;
		return false;
	}
	m_strFlowStatus = Status;
	return true;
}

bool_t Client::CBossTool::Start_Flow(
	const LostArk::Shared::GameplayDataRevision*
		pRequiredDefinitionRevision)
{
	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow || pFlow->Slots.empty())
	{
		m_strFlowStatus = "Save at least one Flow slot before playback.";
		return false;
	}
	const std::string StartSlotId = pFlow->Slots.front().strSlotId;
	return Start_FlowAtSlot(StartSlotId, pRequiredDefinitionRevision);
}

bool_t Client::CBossTool::Start_FlowAtSlot(
	const std::string& strStartSlotId,
	const LostArk::Shared::GameplayDataRevision*
		pRequiredDefinitionRevision)
{
	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow || pFlow->Slots.empty())
	{
		m_strFlowStatus = "Save at least one Flow slot before Restart.";
		return false;
	}
	if (m_FlowDocument.Is_Dirty() ||
		m_FlowDocument.Has_ExternalConflict())
	{
		m_strFlowStatus =
			"Save the Flow before Restart.";
		return false;
	}
	std::string ValidationStatus;
	if (!CValtanPatternFlowDocument::Validate(
			m_FlowDocument.Get_Draft(), Build_AdmittedPatternIds(),
			ValidationStatus))
	{
		m_strFlowStatus =
			"Flow no longer matches the All Patterns inventory: " +
			ValidationStatus;
		return false;
	}
	if (!m_FlowDocument.Verify_SourceRevision(ValidationStatus))
	{
		m_strFlowStatus = "Restart blocked: " + ValidationStatus;
		return false;
	}
	if (strStartSlotId.empty())
	{
		m_strFlowStatus = "The saved gameplay sequence has no first Pattern.";
		return false;
	}
	LostArk::Shared::GameplayDataRevision ExpectedRevision{};
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
	CValtanPatternSoundSourceReadAdmission SoundAdmission;
	std::string RevisionStatus;
	if (!Acquire_ServerPlaybackAdmission(
			ExpectedRevision, PinnedSoundReceipt,
			SoundAdmission, RevisionStatus))
	{
		m_strFlowStatus = std::move(RevisionStatus);
		return false;
	}
	if (nullptr != pRequiredDefinitionRevision &&
		ExpectedRevision != *pRequiredDefinitionRevision)
	{
		m_strFlowStatus =
			"The Server-active Product changed after saved-candidate admission; no Flow command was submitted.";
		return false;
	}
	const CValtanPatternAuditionService& PatternService =
		CValtanPatternAuditionService::Get();
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& PatternPlayback =
		PatternService.Get_Snapshot();
	const bool_t bCanReplaceOwnedPattern =
		(VALTAN_PATTERN_AUDITION_STATE::QUEUED == PatternPlayback.eState ||
		 VALTAN_PATTERN_AUDITION_STATE::ACTIVE == PatternPlayback.eState) &&
		CONSUMER_ID == PatternPlayback.strConsumerId &&
		BOSS_PLACEMENT_ID == PatternPlayback.strBossPlacementId &&
		!PatternService.Get_NextSnapshot().Is_Live() &&
		!PatternService.Has_PendingNextCommand();
	if (CValtanPatternFlowService::Get().Has_PendingStart() ||
		(PatternService.Has_PlaybackOwnership() &&
		 !bCanReplaceOwnedPattern))
	{
		m_strFlowStatus =
			"Resolve the pending Server command before restarting the gameplay sequence.";
		return false;
	}

	m_bRepeat = false;
	m_strRepeatPatternId.clear();
	std::string Status;
	if (!CValtanPatternFlowService::Get().Start(
			BOSS_PLACEMENT_ID,
			*pFlow,
			m_FlowDocument.Get_SourceRevision(),
			strStartSlotId,
			ExpectedRevision,
			PinnedSoundReceipt,
			Status))
	{
		m_strFlowStatus = Status;
		return false;
	}
	m_strFlowStatus = Status;
	return true;
}

bool_t Client::CBossTool::Restart_SavedFlow()
{
	CValtanPatternFlowService& FlowService =
		CValtanPatternFlowService::Get();
	FlowService.Update();
	const VALTAN_PATTERN_FLOW_START_COMMAND& Pending =
		FlowService.Get_PendingStart();
	if (VALTAN_PATTERN_FLOW_START_STATE::UNCONFIRMED == Pending.eState)
	{
		CValtanTuningCommandService& TuningService =
			CValtanTuningCommandService::Get();
		TuningService.Update();
		LostArk::Shared::GameplayDataRevision SavedCandidateRevision{};
		const bool_t bHasSavedGameplayExpectation =
			TuningService.Has_GameplaySourceActivationExpectation();
		if (bHasSavedGameplayExpectation &&
			!TuningService.Try_GetLatestGameplaySourceServerActiveRevision(
				SavedCandidateRevision, m_strFlowStatus))
		{
			m_strFlowStatus =
				"Restart Saved Flow (Fresh Arena) is blocked until the exact Product candidate from the latest canonical Save is Server-active. " +
				m_strFlowStatus;
			return false;
		}
		LostArk::Shared::GameplayDataRevision ExpectedRevision{};
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
		CValtanPatternSoundSourceReadAdmission SoundAdmission;
		std::string Status;
		if (!Acquire_ServerPlaybackAdmission(
				ExpectedRevision, PinnedSoundReceipt,
				SoundAdmission, Status))
		{
			m_strFlowStatus = std::move(Status);
			return false;
		}
		if (bHasSavedGameplayExpectation &&
			ExpectedRevision != SavedCandidateRevision)
		{
			m_strFlowStatus =
				"The Server-active Product changed after saved-candidate admission; no Flow retry was submitted.";
			return false;
		}
		if (Pending.Request.ExpectedDefinitionRevision != ExpectedRevision)
		{
			m_strFlowStatus =
				"The unresolved Flow is pinned to a different Product revision. Re-enter Valtan Arena to clear that ambiguous request, then use Restart Saved Flow (Fresh Arena) again.";
			return false;
		}
		return FlowService.Retry_Start(
			PinnedSoundReceipt, m_strFlowStatus);
	}
	if (FlowService.Has_PendingStart())
	{
		m_strFlowStatus =
			"Restart Saved Flow (Fresh Arena) is waiting for the Server response.";
		return false;
	}
	if (!Reload_FlowDocument())
		return false;

	/* Restart is a disk-backed command. Resolve the activation expectation only
	   after Reload has committed the exact canonical saved baseline that will be
	   copied into FLOW_START. This avoids rejecting a newly saved one-slot Flow
	   from an older in-memory Current Patterns count. */
	CValtanTuningCommandService& TuningService =
		CValtanTuningCommandService::Get();
	TuningService.Update();
	LostArk::Shared::GameplayDataRevision SavedCandidateRevision{};
	const bool_t bHasSavedGameplayExpectation =
		TuningService.Has_GameplaySourceActivationExpectation();
	if (bHasSavedGameplayExpectation &&
		!TuningService.Try_GetLatestGameplaySourceServerActiveRevision(
			SavedCandidateRevision, m_strFlowStatus))
	{
		m_strFlowStatus =
			"Restart Saved Flow (Fresh Arena) is blocked until the exact Product candidate from the latest canonical Save is Server-active. " +
			m_strFlowStatus;
		return false;
	}
	return Start_Flow(
		bHasSavedGameplayExpectation ? &SavedCandidateRevision : nullptr);
}

bool_t Client::CBossTool::Request_RevivePlayer(
	std::string& strOutStatus)
{
	if (nullptr == m_pCommandSink)
	{
		strOutStatus = "Revive is unavailable because the command sink is missing.";
		return false;
	}
	const uint32_t Sequence = 0u == m_iNextReviveSequence ?
		1u : m_iNextReviveSequence;
	m_iNextReviveSequence =
		(std::numeric_limits<uint32_t>::max)() == Sequence ?
			1u : Sequence + 1u;
	m_bReviveFeedbackPending =
		m_pCommandSink->Request_RevivePlayer(Sequence);
	strOutStatus = m_bReviveFeedbackPending ?
		"Revive requested. Waiting for the Server snapshot." :
		"Revive failed: connect the Debug Server and remain in Valtan Arena.";
	return m_bReviveFeedbackPending;
}

bool_t Client::CBossTool::Reload_FlowDocument()
{
	CValtanPatternFlowDocument PreviousFlow = m_FlowDocument;
	m_FlowDocument = CValtanPatternFlowDocument{};
	if (!Reload_Graph())
	{
		m_FlowDocument = std::move(PreviousFlow);
		m_strFlowStatus =
			"Canonical scriptedSequence reload failed; playback and the previous draft were preserved: " +
			m_strStatus;
		return false;
	}
	m_bConfirmDiscardDirtyFlow = false;
	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	m_strSelectedFlowSlotId = nullptr != pFlow && !pFlow->Nodes.empty() ?
		pFlow->strEntryNodeId : std::string{};
	m_strSelectedFlowEdgeId.clear();
	m_strFlowLinkSourceNodeId.clear();
	Normalize_CurrentFlowSelection();
	m_strFlowStatus =
		"Loaded the saved scriptedSequence from Valtan.gameplay.json. Server playback was not changed.";
	return true;
}

bool_t Client::CBossTool::Save_FlowDocument()
{
	std::string MutationStatus;
	if (!Can_MutateCanonicalGraph(MutationStatus))
	{
		m_strFlowStatus = std::move(MutationStatus);
		return false;
	}
	if (nullptr == m_pBalanceTool)
	{
		m_strFlowStatus =
			"Flow save failed: the shared Valtan gameplay draft owner is unavailable.";
		return false;
	}
	const VALTAN_PATTERN_FLOW_DEFINITION* const pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow ||
		!CValtanPatternFlowDocument::Has_LegacyLinearProjection(*pFlow) ||
		pFlow->Slots.empty())
	{
		m_strFlowStatus =
			"Flow save failed: Product scriptedSequence supports one finite ordered route. Remove debug-only branches, repeats, or watchdogs before Save.";
		return false;
	}
	std::vector<std::string> PatternIds;
	PatternIds.reserve(pFlow->Slots.size());
	for (const VALTAN_PATTERN_FLOW_SLOT& Slot : pFlow->Slots)
		PatternIds.push_back(Slot.strPatternId);
	std::string Status;
	if (!m_pBalanceTool->Set_ValtanScriptedSequenceDraft(
			PatternIds, pFlow->iInterStepPursuitMs, Status))
	{
		m_strFlowStatus =
			"Canonical gameplay Save was rejected before the physical transaction: " +
			Status;
		return false;
	}
	if (!m_pBalanceTool->Save_ValtanCanonicalProduct(Status))
	{
		m_strFlowStatus =
			"Canonical gameplay Save failed; Valtan.gameplay.json and every generated Product were preserved: " +
			Status;
		return false;
	}
	const std::string SavedStatus = Status;
	std::string ActivationStatus;
	const bool_t bCanonicalReopened =
		0u != SavedStatus.rfind("COMMIT_SUCCEEDED_REOPEN_FAILED:", 0u);
	const bool_t bActivationPrepared = bCanonicalReopened &&
		m_pBalanceTool->Save_ValtanProduct(ActivationStatus);
	if (!bCanonicalReopened)
	{
		ActivationStatus =
			"Product candidate publication was skipped because the physical commit succeeded but the editor did not reopen that exact revision. Reload Flow, then Save again only if a new draft remains.";
	}
	m_FlowDocument = CValtanPatternFlowDocument{};
	const bool_t bReloaded = Reload_Graph();
	m_bConfirmDiscardDirtyFlow = false;
	Normalize_CurrentFlowSelection();
	m_strFlowStatus =
		std::string("Saved the scriptedSequence and Pattern closure to Valtan.gameplay.json as one canonical revision. ") +
		(bReloaded ?
			"Current Patterns now shows that saved revision. " :
			"The physical Save succeeded, but the editor view could not reload yet; use Load Flow. ") +
		(SavedStatus.empty() ? std::string{} : SavedStatus + " ") +
		(bActivationPrepared ?
			ActivationStatus + " " :
			(bCanonicalReopened ?
				"The physical Save succeeded, but Product candidate publication/activation preparation failed: " +
					ActivationStatus + " " :
				ActivationStatus + " ")) +
		"The running Flow was not changed. Restart Saved Flow (Fresh Arena) is admitted only after that exact saved Product revision is Server-active.";
	return true;
}

bool_t Client::CBossTool::Retry_FlowProductPublishApply()
{
	if (m_FlowDocument.Is_Dirty() ||
		m_FlowDocument.Has_ExternalConflict())
	{
		m_strFlowStatus =
			"Retry Product Publish / Apply requires the saved clean Flow. Save, discard, or reload the current Flow draft first.";
		return false;
	}
	if (nullptr == m_pBalanceTool)
	{
		m_strFlowStatus =
			"Retry Product Publish / Apply is unavailable because the shared Valtan source owner is missing.";
		return false;
	}

	std::string RetryStatus;
	if (!m_pBalanceTool->Retry_ValtanProductPublishApply(RetryStatus))
	{
		m_strFlowStatus = std::move(RetryStatus);
		return false;
	}

	/* Publication does not mutate the saved Flow. If the prior source commit
	   succeeded but a transient Product reopen revoked this window's mutation
	   admission, retry the ordinary read-only graph staging as well. */
	const bool_t bGraphReopened =
		Can_MutateValtanView(m_eGraphAdmission) || Reload_Graph();
	m_strFlowStatus = std::move(RetryStatus);
	if (bGraphReopened)
	{
		m_strFlowStatus +=
			" Canonical Pattern and Flow views are admitted for the current saved revision.";
	}
	else
	{
		m_strFlowStatus +=
			" Product publish/apply retry completed, but the canonical editor graph still could not reopen: " +
			m_strStatus;
	}
	return true;
}

void Client::CBossTool::Synchronize_LiveSelection()
{
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!Boss.isValid || Boss.strPatternId.empty())
	{
		m_strLivePatternId.clear();
		m_strLiveStageId.clear();
		m_strLastAutoRevealedLivePatternId.clear();
		return;
	}
	const VALTAN_PATTERN_VIEW* pPattern =
		Find_AuditionPattern(Boss.strPatternId);
	if (nullptr == pPattern)
	{
		m_strLivePatternId.clear();
		m_strLiveStageId.clear();
		m_strLastAutoRevealedLivePatternId.clear();
		return;
	}
	const VALTAN_STAGE_VIEW* pStage = Find_LiveStage(*pPattern);
	m_strLivePatternId = pPattern->strPatternId;
	m_strLiveStageId = nullptr == pStage ? std::string{} : pStage->strStageId;
}

void Client::CBossTool::Render()
{
	if (!m_bOpen)
		return;
	if (!m_bGraphLoadAttempted)
		(void)Reload_Graph();

	ImGui::SetNextWindowPos(ImVec2(36.f, 48.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(980.f, 690.f), ImGuiCond_FirstUseEver);
	if (m_bFocusPending)
	{
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
		ImGui::SetNextWindowFocus();
		m_bFocusPending = false;
	}
	if (!ImGui::Begin("Valtan Boss Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::TextDisabled(
		"Server pattern verification and canonical ordered gameplay playback.");
	if (ImGui::BeginTabBar("##bossToolTabs"))
	{
		if (ImGui::BeginTabItem("Boss Verification"))
		{
			Render_BossVerificationTab();
			ImGui::EndTabItem();
		}
		const ImGuiTabItemFlags FlowFlags = m_bSelectPatternFlowTab ?
			ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
		if (ImGui::BeginTabItem("Pattern Flow", nullptr, FlowFlags))
		{
			Render_PatternFlowTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Logic Flow"))
		{
			Render_LogicFlowTab();
			ImGui::EndTabItem();
		}
		m_bSelectPatternFlowTab = false;
		ImGui::EndTabBar();
	}

	ImGui::End();
}

void Client::CBossTool::Render_LogicPatternWindow()
{
	if (!m_bLogicPatternOpen)
		return;
	if (!m_bGraphLoadAttempted)
		(void)Reload_Graph();

	ImGui::SetNextWindowPos(ImVec2(52.f, 36.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(1420.f, 840.f), ImGuiCond_FirstUseEver);
	if (m_bLogicPatternFocusPending)
	{
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
		ImGui::SetNextWindowFocus();
		m_bLogicPatternFocusPending = false;
	}
	if (!ImGui::Begin("Valtan Logic Pattern", &m_bLogicPatternOpen))
	{
		ImGui::End();
		return;
	}
	ImGui::TextDisabled(
		"Read-only blueprint over the Boss Tool's one admitted Pattern graph and replicated Server cursor.");
	Render_LogicPatternContent();
	ImGui::End();
}

void Client::CBossTool::Render_BossVerificationTab()
{
	Render_LiveSummary();
	Render_ActionBar();
	ImGui::Separator();

	if (!Can_DisplayValtanView(m_eGraphAdmission))
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (ImGui::Button("Retry Graph Load"))
			(void)Reload_Graph();
		return;
	}
	if (!m_bGraphReady && m_bProductFallbackReady &&
		Can_DisplayValtanView(m_eGraphAdmission))
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.72f, 0.18f, 1.f),
			"READ-ONLY PRODUCT FALLBACK");
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (ImGui::Button("Retry Strict Graph Load##productFallback"))
			(void)Reload_Graph();
		ImGui::Separator();
		if (ImGui::BeginTable(
				"##bossToolProductFallbackLayout", 2,
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_BordersInnerV |
				ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn(
				"Generated Product Patterns",
				ImGuiTableColumnFlags_WidthFixed, 330.f);
			ImGui::TableSetupColumn(
				"Read-only Product Detail",
				ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			Render_ProductFallbackPatternList();
			ImGui::TableSetColumnIndex(1);
			Render_ProductFallbackSelectedPattern();
			ImGui::EndTable();
		}
		return;
	}
	if (!Can_MutateValtanView(m_eGraphAdmission))
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (ImGui::Button("Retry Graph Load##preserved"))
			(void)Reload_Graph();
		ImGui::Separator();
	}

	if (ImGui::BeginTable(
			"##bossToolLayout",
			2,
			ImGuiTableFlags_Resizable |
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn(
			"Patterns", ImGuiTableColumnFlags_WidthFixed, 330.f);
		ImGui::TableSetupColumn(
			"Selected", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if (ImGui::BeginTabBar("##bossVerificationPatternTabs"))
		{
			if (ImGui::BeginTabItem("All Patterns"))
			{
				Render_PatternList();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Current Patterns"))
			{
				Render_CurrentPatternList();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::TableSetColumnIndex(1);
		Render_SelectedPattern();
		ImGui::EndTable();
	}
}

void Client::CBossTool::Render_LogicFlowTab()
{
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	ImGui::TextWrapped(
		"The canonical Logic canvas now lives in its own large Logic Pattern window. This tab keeps only the Boss Tool context and deep link.");
	if (Boss.isValid && !Boss.strPatternId.empty())
	{
		const VALTAN_PATTERN_VIEW* const pLivePattern =
			Find_AuditionPattern(Boss.strPatternId);
		const ENCOUNTER_PATTERN_REFERENCE* const pFallbackPattern =
			nullptr == pLivePattern ?
				Find_ProductFallbackPattern(Boss.strPatternId) : nullptr;
		const ENCOUNTER_STAGE_REFERENCE* const pFallbackStage =
			nullptr == pFallbackPattern ? nullptr :
				Find_ProductFallbackStage(*pFallbackPattern, Boss.strActionId);
		const bool_t bHasCounterHitBranch =
			(nullptr != pLivePattern &&
			 CBossLogicFlowViewModel::Has_CounterHitBranch(*pLivePattern)) ||
			(nullptr != pFallbackPattern && std::any_of(
				pFallbackPattern->stages.begin(), pFallbackPattern->stages.end(),
				[](const ENCOUNTER_STAGE_REFERENCE& Stage)
				{ return Stage.bHasCounterHitBranch; }));
		const std::string& DisplayName =
			nullptr != pLivePattern && !pLivePattern->strDisplayName.empty() ?
				pLivePattern->strDisplayName :
				(nullptr != pFallbackPattern &&
				 !pFallbackPattern->displayName.empty() ?
					pFallbackPattern->displayName : Boss.strPatternId);
		ImGui::Text("Server live: %s | Sequence %u",
			DisplayName.c_str(), Boss.iPatternSequence);
		Render_LogicCounterBadge(bHasCounterHitBranch);
		ImGui::SameLine();
		ImGui::Text("| Stage %u", Boss.iPatternStageIndex);
		if (nullptr != pFallbackPattern)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.72f, 0.18f, 1.f),
				"READ-ONLY PRODUCT FALLBACK");
		}
		ImGui::TextWrapped("Action: %s%s%s",
			Boss.strActionId.empty() ? "None" : Boss.strActionId.c_str(),
			nullptr == pFallbackStage ? "" : " | Product Stage: ",
			nullptr == pFallbackStage ? "" : pFallbackStage->stageId.c_str());
	}
	else
	{
		ImGui::TextDisabled("Server live: IDLE");
	}
	const auto& History = m_LogicFlowObservedEdges.Get_History();
	if (!History.empty())
	{
		const BOSS_LOGIC_FLOW_OBSERVED_EDGE& Latest = History.back();
		ImGui::TextWrapped(
			"Latest fail-closed observation: %s / %s",
			Latest.strSourceStageId.c_str(), Latest.strOutcome.c_str());
	}
	if (ImGui::Button("Open Large Logic Pattern Window"))
	{
		Open_LogicPattern();
		m_bLogicPatternOpenRequest = true;
	}
	ImGui::TextDisabled(
		"F1 > Logic Pattern opens the same window directly; no graph or playback owner is cloned.");
}

void Client::CBossTool::Render_LogicPatternContent()
{
	if (!Can_DisplayValtanView(m_eGraphAdmission))
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (ImGui::Button("Retry Graph Load##logicPattern"))
			(void)Reload_Graph();
		return;
	}
	if (!m_bGraphReady && m_bProductFallbackReady)
	{
		Render_ProductFallbackLogicPattern();
		return;
	}
	if (!Can_MutateValtanView(m_eGraphAdmission))
	{
		ImGui::TextWrapped(
			"Logic Pattern is showing the previous admitted graph read-only: %s",
			m_strStatus.c_str());
	}

	const VALTAN_PATTERN_FLOW_SNAPSHOT& FlowPlayback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	const bool_t bFlowLifecycleMatchesBoss =
		FlowPlayback.Is_InFlight() && Boss.isValid &&
		!FlowPlayback.strCurrentPatternId.empty() &&
		FlowPlayback.strCurrentPatternId == Boss.strPatternId &&
		0u != FlowPlayback.iPatternSequence &&
		FlowPlayback.iPatternSequence == Boss.iPatternSequence;
	if (FlowPlayback.Is_InFlight())
	{
		ImGui::TextWrapped(
			"Current Flow: %s | Slot %u / %u | %s",
			FlowPlayback.strFlowId.empty() ? "Server Flow" :
				FlowPlayback.strFlowId.c_str(),
			static_cast<uint32_t>(FlowPlayback.iCurrentSlotOrdinal),
			static_cast<uint32_t>(FlowPlayback.iSlotCount),
			bFlowLifecycleMatchesBoss ? "LIVE SYNC" : "WAITING FOR LIVE SYNC");
	}

	std::string PatternId;
	bool_t bLivePattern = false;
	if (Boss.isValid && !Boss.strPatternId.empty())
	{
		PatternId = Boss.strPatternId;
		bLivePattern = true;
	}
	else if (!FlowPlayback.strCurrentPatternId.empty())
	{
		PatternId = FlowPlayback.strCurrentPatternId;
	}
	else
	{
		PatternId = m_strSelectedPatternId;
	}
	const VALTAN_PATTERN_VIEW* const pPattern =
		Find_AuditionPattern(PatternId);
	if (nullptr == pPattern)
	{
		ImGui::TextWrapped("Current Pattern: %s",
			PatternId.empty() ? "None" : PatternId.c_str());
		ImGui::TextDisabled(
			"Start a Server Pattern or select one in Boss Verification to inspect its admitted blueprint.");
		return;
	}
	if (!Project_LogicFlowView(*pPattern))
	{
		ImGui::TextWrapped("%s", m_strLogicFlowStatus.c_str());
		return;
	}

	BOSS_LOGIC_FLOW_RENDER_CONTEXT Context;
	if (m_LogicFlowSelection.strPatternId == pPattern->strPatternId &&
		BOSS_LOGIC_FLOW_SELECTION_KIND::NONE != m_LogicFlowSelection.eKind)
	{
		Context.strSelectedStageId = m_LogicFlowSelection.strStageId;
	}
	else if (pPattern->strPatternId == m_strSelectedPatternId)
	{
		Context.strSelectedStageId = m_strSelectedStageId;
	}
	Context.strLiveActionId = bLivePattern ?
		std::string_view(Boss.strActionId) : std::string_view{};
	const auto& History = m_LogicFlowObservedEdges.Get_History();
	Context.pObservedEdge = History.empty() ? nullptr : &History.back();
	Context.bLivePattern = bLivePattern;
	Context.bAllowSelection = true;
	Context.fMinimumCanvasHeight = 560.f;

	if (ImGui::BeginTable("##LogicPatternWorkspace", 2,
			ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_SizingStretchProp,
			ImGui::GetContentRegionAvail()))
	{
		ImGui::TableSetupColumn("Blueprint", ImGuiTableColumnFlags_WidthStretch,
			0.72f);
		ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthStretch,
			0.28f);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::BeginChild("##LogicPatternCanvasPane", ImVec2(0.f, 0.f), false,
			ImGuiWindowFlags_NoScrollbar);
		BOSS_LOGIC_FLOW_SELECTION Selection;
		if (CBossLogicFlowRenderer::Render(
				m_LogicFlowView, Context, m_LogicFlowCanvasState, Selection))
		{
			m_LogicFlowSelection = std::move(Selection);
		}
		ImGui::EndChild();

		ImGui::TableNextColumn();
		ImGui::BeginChild("##LogicPatternInspectorPane", ImVec2(0.f, 0.f),
			false);
		Render_LogicPatternInspector(*pPattern, Boss, bLivePattern);
		ImGui::EndChild();
		ImGui::EndTable();
	}
}

void Client::CBossTool::Render_LogicPatternInspector(
	const VALTAN_PATTERN_VIEW& Pattern,
	const HUD_BOSS_STATE& Boss,
	const bool_t bLivePattern)
{
	ImGui::SeparatorText("Live snapshot");
	if (bLivePattern)
	{
		ImGui::TextWrapped("Pattern %s", Boss.strPatternId.c_str());
		ImGui::Text("Sequence %u", Boss.iPatternSequence);
		Render_LogicCounterBadge(m_LogicFlowView.bHasCounterHitBranch);
		ImGui::SameLine();
		ImGui::Text("| Stage %u | Tick %u",
			Boss.iPatternStageIndex, Boss.iServerTick);
		ImGui::TextWrapped("Action %s", Boss.strActionId.c_str());
		if (Boss.iResponseThreshold > 0u)
		{
			ImGui::TextWrapped(
				"Response: %u / %u confirmed HP damage",
				Boss.iResponseProgress, Boss.iResponseThreshold);
		}
		const bool_t bCounterable = LostArk::Shared::Has_BossCombatFlag(
			Boss.iBossCombatFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::COUNTERABLE);
		const bool_t bGroggy = LostArk::Shared::Has_BossCombatFlag(
			Boss.iBossCombatFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::GROGGY);
		ImGui::TextDisabled("COUNTERABLE %s | GROGGY %s",
			bCounterable ? "ON" : "OFF", bGroggy ? "ON" : "OFF");
	}
	else
	{
		ImGui::TextDisabled("Sequence --");
		Render_LogicCounterBadge(m_LogicFlowView.bHasCounterHitBranch);
		ImGui::TextDisabled("No matching live Server Pattern.");
	}

	std::string InspectorActionId;
	if (m_LogicFlowSelection.strPatternId == Pattern.strPatternId &&
		!m_LogicFlowSelection.strActionId.empty())
	{
		InspectorActionId = m_LogicFlowSelection.strActionId;
	}
	else if (bLivePattern)
	{
		InspectorActionId = Boss.strActionId;
	}
	else if (Pattern.strPatternId == m_strSelectedPatternId)
	{
		const VALTAN_STAGE_VIEW* const pSelected = Find_SelectedStage(Pattern);
		if (nullptr != pSelected)
			InspectorActionId = pSelected->strActionId;
	}
	const auto NodeAt = std::find_if(
		m_LogicFlowView.Nodes.begin(), m_LogicFlowView.Nodes.end(),
		[&InspectorActionId](const BOSS_LOGIC_FLOW_NODE_VIEW& Node)
		{ return Node.strActionId == InspectorActionId; });
	ImGui::SeparatorText("Stage conditions");
	if (m_LogicFlowView.Nodes.end() == NodeAt)
	{
		ImGui::TextDisabled("Click a Stage or branch to inspect it.");
	}
	else
	{
		ImGui::Text("%s | %s", NodeAt->strStageKind.c_str(),
			NodeAt->strStageId.c_str());
		ImGui::TextWrapped("%s", NodeAt->strActionId.c_str());
		if (NodeAt->ConditionSummaries.empty())
			ImGui::TextDisabled("No authored conditional input on this Stage.");
		else
		{
			for (const std::string& Summary : NodeAt->ConditionSummaries)
				ImGui::BulletText("%s", Summary.c_str());
		}
		ImGui::TextDisabled("Counter contract: %s",
			NodeAt->bCounterWindow ? "CLOSED" :
				(NodeAt->bCounterContractIncomplete ? "INCOMPLETE" : "N/A"));

		ImGui::SeparatorText("Outgoing branches");
		for (const ACTION_COMPOSITION_GRAPH_EDGE& Edge :
			m_LogicFlowView.Graph.Edges)
		{
			if (Edge.strSourceActionId != NodeAt->strActionId)
				continue;
			std::string Destination = !Edge.strTargetPatternId.empty() ?
				("Pattern " + Edge.strTargetPatternId) :
				(!Edge.strTargetActionId.empty() ? Edge.strTargetActionId : "END");
			ImGui::BulletText("%s -> %s%s", Edge.strOutcome.c_str(),
				Destination.c_str(),
				ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::DERIVED_TIMEOUT ==
					Edge.eOrigin ? " (derived)" : "");
		}
	}
	if (BOSS_LOGIC_FLOW_SELECTION_KIND::BRANCH ==
		m_LogicFlowSelection.eKind &&
		m_LogicFlowSelection.strPatternId == Pattern.strPatternId)
	{
		ImGui::SeparatorText("Selected branch");
		ImGui::Text("Outcome: %s", m_LogicFlowSelection.strOutcome.c_str());
		if (!m_LogicFlowSelection.strTargetPatternId.empty())
			ImGui::TextWrapped("Target Pattern: %s",
				m_LogicFlowSelection.strTargetPatternId.c_str());
		else if (!m_LogicFlowSelection.strTargetActionId.empty())
			ImGui::TextWrapped("Target Action: %s",
				m_LogicFlowSelection.strTargetActionId.c_str());
		else
			ImGui::TextDisabled("Target: END");
	}

	ImGui::SeparatorText("Observed edge history");
	ImGui::TextDisabled(
		"Only consecutive Server ticks plus one unique graph edge are admitted. Gaps and ambiguous targets stay blank.");
	const auto& History = m_LogicFlowObservedEdges.Get_History();
	if (History.empty())
	{
		ImGui::TextDisabled("No fail-closed edge observed yet.");
	}
	else
	{
		if (ImGui::SmallButton("Clear observed history"))
			m_LogicFlowObservedEdges.Reset();
		for (auto At = History.rbegin(); At != History.rend(); ++At)
		{
			const std::string Destination = !At->strTargetPatternId.empty() ?
				("Pattern " + At->strTargetPatternId) :
				(!At->strTargetActionId.empty() ? At->strTargetActionId : "END");
			ImGui::PushID(static_cast<int>(At->iObservationSequence));
			ImGui::TextWrapped("#%llu  tick %u -> %u",
				static_cast<unsigned long long>(At->iObservationSequence),
				At->iSourceServerTick, At->iTargetServerTick);
			ImGui::TextWrapped("%s / %s -> %s",
				At->strSourceStageId.c_str(), At->strOutcome.c_str(),
				Destination.c_str());
			ImGui::Separator();
			ImGui::PopID();
		}
	}
}

bool_t Client::CBossTool::Project_LogicFlowView(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	if (m_LogicFlowView.strPatternId == Pattern.strPatternId &&
		m_LogicFlowView.Graph.iSourceGeneration ==
			m_iLogicFlowSourceGeneration)
	{
		return true;
	}
	ACTION_COMPOSITION_GRAPH_ERROR Error;
	if (!CBossLogicFlowViewModel::Project(
			Pattern, m_iLogicFlowSourceGeneration, m_LogicFlowView, Error))
	{
		m_strLogicFlowStatus = Error.strMessage.empty() ?
			"Logic Pattern projection failed." : Error.strMessage;
		return false;
	}
	if (m_LogicFlowSelection.strPatternId != Pattern.strPatternId)
		m_LogicFlowSelection.Clear();
	m_strLogicFlowStatus =
		"Logic Pattern projected from the admitted Pattern graph.";
	return true;
}

bool_t Client::CBossTool::Fail_GraphReload(
	const std::string& strStrictFailure,
	CValtanCanonicalProductReadAdmission* const pCanonicalAdmission)
{
	if (m_bGraphReady)
	{
		/* Never replace a fully joined display snapshot with the reduced Product
		   projection.  Its command admission was revoked by Reload_Graph(), so the
		   preserved graph remains diagnosis-only until a strict reload succeeds. */
		m_eGraphAdmission = VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
		m_strStatus =
			"Canonical graph STALE_PRESERVED; previous rows are display-only: " +
			strStrictFailure;
		return false;
	}
	if (nullptr == pCanonicalAdmission)
	{
		/* Acquire failure (including a publisher writer window) provides no
		   generation pin.  Never open Product files outside an admission and risk
		   displaying a mixed transaction. */
		m_eGraphAdmission = m_bProductFallbackReady ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::REJECTED;
		m_strStatus = m_bProductFallbackReady ?
			"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED: canonical Product read admission failed, so the prior fallback generation was preserved and no files were reopened. Commands remain blocked. Failure: " +
				strStrictFailure :
			"Graph reload failed before canonical Product read admission; no unpinned fallback read was attempted: " +
				strStrictFailure;
		return false;
	}

	CEncounterPatternReference StagedProduct;
	std::string ProductStatus;
	if (StagedProduct.Load(
			CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json"),
			ProductStatus))
	{
		std::string CurrentStatus;
		if (!pCanonicalAdmission->Validate_StillCurrent(CurrentStatus))
		{
			Schedule_CanonicalReloadRetry();
			m_eGraphAdmission = m_bProductFallbackReady ?
				VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
				VALTAN_VIEW_ADMISSION::REJECTED;
			m_strStatus = m_bProductFallbackReady ?
				"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED: Product generation changed before fallback commit; the prior fallback was preserved. Commands remain blocked. Strict failure: " +
					strStrictFailure + " | Product admission failure: " +
					CurrentStatus :
				"Graph reload failed and the generated Product fallback generation changed before commit; no fallback was published. Strict failure: " +
					strStrictFailure + " | Product admission failure: " +
					CurrentStatus;
			return false;
		}
		m_ProductFallbackEncounterReference = std::move(StagedProduct);
		m_bProductFallbackReady = true;
		m_eGraphAdmission = VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
		const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
		const ENCOUNTER_PATTERN_REFERENCE* pSelected =
			m_ProductFallbackEncounterReference.Find_Pattern(
				m_strSelectedPatternId);
		if (nullptr == pSelected && Boss.isValid)
		{
			pSelected = m_ProductFallbackEncounterReference.Find_Pattern(
				Boss.strPatternId);
		}
		if (nullptr == pSelected &&
			!m_ProductFallbackEncounterReference.Get_Patterns().empty())
		{
			pSelected =
				&m_ProductFallbackEncounterReference.Get_Patterns().front();
		}
		m_strSelectedPatternId = nullptr == pSelected ?
			std::string{} : pSelected->patternId;
		m_strSelectedStageId = nullptr == pSelected || pSelected->stages.empty() ?
			std::string{} : pSelected->stages.front().stageId;
		m_strStatus =
			"READ-ONLY PRODUCT FALLBACK: strict split authoring join failed; " +
			std::to_string(
				m_ProductFallbackEncounterReference.Get_Patterns().size()) +
			" generated Product patterns remain visible. Save, Restart, Play, "
			"Repeat, and Next stay blocked. Strict failure: " +
			strStrictFailure;
		return false;
	}

	if (m_bProductFallbackReady)
	{
		m_eGraphAdmission = VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
		m_strStatus =
			"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED: strict split authoring "
			"join and Product fallback refresh both failed. Commands remain "
			"blocked. Strict failure: " + strStrictFailure +
			" | Product fallback failure: " + ProductStatus;
		return false;
	}

	m_eGraphAdmission = VALTAN_VIEW_ADMISSION::REJECTED;
	m_strStatus = "Graph reload failed: " + strStrictFailure +
		" | Product fallback failed: " + ProductStatus;
	return false;
}

void Client::CBossTool::Update_LogicFlowObservation()
{
	BOSS_LOGIC_FLOW_LIVE_SNAPSHOT Snapshot;
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	Snapshot.bValid = Can_DisplayValtanView(m_eGraphAdmission) &&
		m_bGraphReady && Boss.isValid && 0u != Boss.iCurrentHp &&
		!Boss.strPatternId.empty() && !Boss.strActionId.empty();
	Snapshot.iServerTick = Boss.iServerTick;
	Snapshot.iPatternSequence = Boss.iPatternSequence;
	Snapshot.strPatternId = Boss.strPatternId;
	Snapshot.strActionId = Boss.strActionId;

	const BOSS_LOGIC_FLOW_VIEW* pCurrentView = nullptr;
	if (Snapshot.bValid)
	{
		const VALTAN_PATTERN_VIEW* const pPattern =
			Find_AuditionPattern(Snapshot.strPatternId);
		if (nullptr != pPattern && Project_LogicFlowView(*pPattern))
			pCurrentView = &m_LogicFlowView;
	}
	(void)m_LogicFlowObservedEdges.Observe(
		Snapshot, pCurrentView, nullptr);
}

void Client::CBossTool::Render_PatternFlowTab()
{
	const VALTAN_PATTERN_FLOW_DEFINITION* const pCurrentFlow =
		m_FlowDocument.Get_DefaultFlow();
	const bool_t bHasServerProjection = nullptr != pCurrentFlow &&
		CValtanPatternFlowDocument::Has_LegacyLinearProjection(*pCurrentFlow);
	ImGui::TextDisabled(bHasServerProjection ?
		"Pattern Play/Restart in Boss Verification runs one selected Pattern from Stage 1 and keeps the current arena. Restart Saved Flow here reloads the complete scriptedSequence, restores the arena, starts Pattern 01, then follows every saved Next Pattern and Wait." :
		"Product Save supports one finite ordered scriptedSequence; remove debug-only routing before Save.");
	if (!Can_MutateValtanView(m_eGraphAdmission))
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (ImGui::Button("Retry Graph Load##flow"))
			(void)Reload_Graph();
	}

	const CValtanPatternFlowService& FlowService = CValtanPatternFlowService::Get();
	const VALTAN_PATTERN_FLOW_START_COMMAND& PendingFlowStart =
		FlowService.Get_PendingStart();
	const bool_t bPatternPlaybackOwnership =
		CValtanPatternAuditionService::Get().Has_PlaybackOwnership();
	const bool_t bOtherCommandPending =
		Is_ServerArenaPresetPending() || bPatternPlaybackOwnership;
	const bool_t bDocumentCommandPending =
		FlowService.Has_PendingStart() || bOtherCommandPending;
	const bool_t bRestartWaiting =
		VALTAN_PATTERN_FLOW_START_STATE::WAITING_VERDICT ==
			PendingFlowStart.eState || bOtherCommandPending;
	const char_t* pDocumentState = !m_FlowDocument.Is_Ready() ?
		"NOT LOADED" :
		(m_FlowDocument.Has_ExternalConflict() ? "EXTERNAL CONFLICT" :
			(m_FlowDocument.Is_Dirty() ? "UNSAVED" : "SAVED"));
	ImGui::Text("Flow: %s", pDocumentState);
	ImGui::SameLine();
	ImGui::BeginDisabled(
		bDocumentCommandPending ||
		!Can_MutateValtanView(m_eGraphAdmission));
	if (ImGui::Button("Load Flow"))
	{
		if (m_FlowDocument.Is_Dirty())
		{
			m_bConfirmDiscardDirtyFlow = true;
			m_strFlowStatus =
				"Load discards this draft and reads scriptedSequence from Valtan.gameplay.json. Confirm below.";
		}
		else
		{
			(void)Reload_FlowDocument();
		}
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Load the canonical gameplay scriptedSequence from disk.");
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!m_FlowDocument.Is_Ready() || !m_FlowDocument.Is_Dirty());
	if (ImGui::Button("Save Flow"))
		(void)Save_FlowDocument();
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool_t bCleanFlowProductRetry =
		!m_FlowDocument.Is_Dirty() &&
		!m_FlowDocument.Has_ExternalConflict();
	ImGui::BeginDisabled(
		bDocumentCommandPending || nullptr == m_pBalanceTool ||
		!bCleanFlowProductRetry);
	if (ImGui::Button("Retry Product Publish / Apply"))
		(void)Retry_FlowProductPublishApply();
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Retry only Product reopen, candidate publication, and runtime apply for the clean saved Flow. Canonical source files are not saved again.");
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(
		bDocumentCommandPending ||
		!Can_MutateValtanView(m_eGraphAdmission) ||
		!m_FlowDocument.Is_Ready() ||
		!m_FlowDocument.Is_Dirty());
	if (ImGui::Button("Discard Changes..."))
	{
		m_bConfirmDiscardDirtyFlow = true;
		m_strFlowStatus =
			"Discard restores the last saved gameplay scriptedSequence and removes every unsaved order, route, wait, and limit edit. Confirm below.";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		bRestartWaiting ||
		!Can_MutateValtanView(m_eGraphAdmission) ||
		m_FlowDocument.Is_Dirty());
	if (ImGui::Button("Restart Saved Flow (Fresh Arena)"))
		(void)Restart_SavedFlow();
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip(
			"Reload the complete saved scriptedSequence from Valtan.gameplay.json, restore the authoritative walls, floors, props, collision, Nav, and combat objects, then start saved Pattern 01 and follow every saved Next Pattern and inter-pattern Wait.");
	if (m_bConfirmDiscardDirtyFlow)
	{
		ImGui::TextWrapped(
			"Discard every unsaved sequence change and restore scriptedSequence from Valtan.gameplay.json?");
		ImGui::BeginDisabled(
			bDocumentCommandPending ||
			!Can_MutateValtanView(m_eGraphAdmission));
		if (ImGui::Button("Discard Changes##ConfirmFlowDiscard"))
			(void)Reload_FlowDocument();
		ImGui::SameLine();
		if (ImGui::Button("Keep Draft"))
		{
			m_bConfirmDiscardDirtyFlow = false;
			m_strFlowStatus = "Kept the current unsaved gameplay sequence draft.";
		}
		ImGui::EndDisabled();
	}

	if (!m_strFlowStatus.empty())
		ImGui::TextWrapped("%s", m_strFlowStatus.c_str());
	ImGui::Checkbox("Pattern Route Editor", &m_bFlowGraphEditor);
	ImGui::SameLine();
	ImGui::TextDisabled(m_bFlowGraphEditor ?
		"Choose Pattern order, the next Pattern, and the wait between them." :
		"Edit the simple ordered list.");

	/* Keep Next independent of draft admission and selection. Small windows
	   scroll this parent; both right-side children retain usable minimums. */
	const float fNextCardHeight = 280.f;
	const float fColumnHeight = (std::max)(
		480.f, ImGui::GetContentRegionAvail().y);
	const float fSelectedHeight = fColumnHeight - fNextCardHeight -
		ImGui::GetStyle().ItemSpacing.y;
	if (ImGui::BeginTable(
			"##bossPatternFlowLayout",
			2,
			ImGuiTableFlags_Resizable |
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn(
			m_bFlowGraphEditor ? "Pattern Route" : "Ordered Patterns",
			ImGuiTableColumnFlags_WidthFixed, 430.f);
		ImGui::TableSetupColumn(
			m_bFlowGraphEditor ? "Pattern / Next Settings" : "Selected Pattern",
			ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if (ImGui::BeginChild("##bossFlowSlotsPane", ImVec2(0.f, fColumnHeight)))
		{
			if (Can_DisplayValtanView(m_eGraphAdmission) &&
				m_bGraphReady && m_FlowDocument.Is_Ready())
			{
				if (m_bFlowGraphEditor)
					Render_FlowGraphEditor();
				else
					Render_FlowSlotList();
			}
			else
				ImGui::TextWrapped("Flow slots are unavailable. Next runtime state is still visible on the right.");
		}
		ImGui::EndChild();
		ImGui::TableSetColumnIndex(1);
		if (ImGui::BeginChild("##bossFlowSelectedPane", ImVec2(0.f, fSelectedHeight), true))
		{
			if (Can_DisplayValtanView(m_eGraphAdmission) &&
				m_bGraphReady && m_FlowDocument.Is_Ready())
				Render_FlowSelectedSlot();
			else
				ImGui::TextWrapped("%s", m_strFlowStatus.c_str());
		}
		ImGui::EndChild();
		if (ImGui::BeginChild("##bossNextPatternCard", ImVec2(0.f, fNextCardHeight), true))
			Render_NextPatternCard();
		ImGui::EndChild();
		ImGui::EndTable();
	}
}

void Client::CBossTool::Render_NextPatternCard()
{
	CValtanPatternAuditionService& Service = CValtanPatternAuditionService::Get();
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Current = Service.Get_Snapshot();
	const VALTAN_NEXT_PATTERN_SNAPSHOT& Next = Service.Get_NextSnapshot();
	const VALTAN_NEXT_PATTERN_COMMAND& Command = Service.Get_NextCommand();
	const bool_t bFlowActive = CValtanPatternFlowService::Get().Get_Snapshot().Is_InFlight();
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	const auto LabelFor = [this](const std::string& Id)
	{
		const VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(Id);
		return nullptr != pPattern && !pPattern->strDisplayName.empty() ?
			pPattern->strDisplayName : (Id.empty() ? std::string("None") : Id);
	};
	ImGui::SeparatorText("Next Pattern (no reset)");
	ImGui::TextWrapped("Current: %s | %s",
		LabelFor(Boss.isValid ? Boss.strPatternId : Current.strPatternId).c_str(),
		Boss.isValid && !Boss.strPatternId.empty() ? "SERVER" :
			Describe_ValtanPatternAuditionState(Current.eState));
	ImGui::TextWrapped("Next: %s | %s", LabelFor(Next.strPatternId).c_str(),
		Describe_ValtanNextPatternState(Next.eState));
	if (Next.Is_Live())
	{
		ImGui::TextWrapped("Reservation %u | epoch %u | predecessor %u -> %u",
			Next.iRequestSequence, Next.iRoomAuditionEpoch,
			Next.iPredecessorPatternSequence, Next.iExpectedPatternSequence);
		if (Next.bReservationConsumed)
			ImGui::TextWrapped("Reservation consumed: this occurrence is awaiting ACTIVE. Choose or cancel becomes available for the next reservation after it starts.");
	}
	if (!Next.strStatus.empty())
		ImGui::TextWrapped("%s", Next.strStatus.c_str());
	if (!Command.strStatus.empty())
		ImGui::TextWrapped("Command: %s | %s", Describe_ValtanNextCommandState(Command.eState),
			Command.strStatus.c_str());

	std::string SelectionStatus;
	LostArk::Shared::GameplayDataRevision ExpectedNextRevision{};
	const bool_t bRevisionAdmitted = Observe_ServerActivePatternRevision(
		ExpectedNextRevision, SelectionStatus);
	const bool_t bRuntimeCanChoose = bRevisionAdmitted &&
		Service.Can_QueueNextPattern(
			BOSS_PLACEMENT_ID, ExpectedNextRevision, SelectionStatus);
	const bool_t bCanChoose = Can_MutateValtanView(m_eGraphAdmission) &&
		m_bNextPatternInventoryReady &&
		!m_NextPatternIds.empty() && bRuntimeCanChoose;
	ImGui::BeginDisabled(!bCanChoose);
	if (ImGui::Button("Next Pattern..."))
		ImGui::OpenPopup("##chooseBossNextPattern");
	ImGui::EndDisabled();
	Render_NextPatternPicker();
	ImGui::BeginDisabled(!Next.Is_Live() || Next.bReservationConsumed || Service.Has_PendingNextCommand());
	if (ImGui::Button("Cancel Reservation"))
	{
		std::string MutationStatus;
		if (!Can_MutateCanonicalGraph(MutationStatus))
			m_strNextPatternStatus = std::move(MutationStatus);
		else
			(void)Service.Clear_NextPattern(m_strNextPatternStatus);
	}
	ImGui::EndDisabled();
	if (VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED == Command.eState)
	{
		if (ImGui::Button("Retry Same Next Command"))
		{
			LostArk::Shared::GameplayDataRevision ExpectedRevision{};
			VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
			CValtanPatternSoundSourceReadAdmission SoundAdmission;
			std::string RevisionStatus;
			if (!Acquire_ServerPlaybackAdmission(
					ExpectedRevision, PinnedSoundReceipt,
					SoundAdmission, RevisionStatus))
				m_strNextPatternStatus = std::move(RevisionStatus);
			else if (Command.Request.ExpectedDefinitionRevision !=
				ExpectedRevision)
			{
				m_strNextPatternStatus =
					"Next retry rejected because the active Pattern changed. Choose Next again.";
			}
			else
				(void)Service.Retry_NextPatternCommand(
					PinnedSoundReceipt, m_strNextPatternStatus);
		}
	}
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	if (Next.Is_Live() && Player.isValid && 0u == Player.iCurrentHp)
	{
		ImGui::BeginDisabled(nullptr == m_pCommandSink);
		if (ImGui::Button("Revive Player##next"))
			(void)Request_RevivePlayer(m_strNextPatternStatus);
		ImGui::EndDisabled();
	}
	if (!Can_MutateValtanView(m_eGraphAdmission))
	{
		ImGui::TextWrapped(
			"Preserved Next rows are display-only until canonical reload is ADMITTED.");
		if (ImGui::Button("Reload Graph##next"))
			(void)Reload_Graph();
	}
	else if (!m_bNextPatternInventoryReady)
	{
		ImGui::TextWrapped("New selection is unavailable until the canonical graph reload succeeds. Reservation controls remain available.");
		if (ImGui::Button("Reload Graph##next"))
			(void)Reload_Graph();
	}
	if (!bRuntimeCanChoose)
		ImGui::TextWrapped("%s", SelectionStatus.c_str());
	ImGui::TextWrapped("%s", m_strNextPatternStatus.c_str());
	ImGui::TextWrapped("Next starts after the current Server pattern without resetting the arena. Repeat is turned off.");
	if (bFlowActive)
		ImGui::TextWrapped("Choosing Next stops the remaining Flow order after this occurrence; the saved slots are unchanged.");
}

void Client::CBossTool::Render_NextPatternPicker()
{
	ImGui::SetNextWindowSize(ImVec2(470.f, 440.f), ImGuiCond_FirstUseEver);
	if (!ImGui::BeginPopup("##chooseBossNextPattern"))
		return;
	ImGui::Text("Saved Boss Patterns: %zu", m_NextPatternIds.size());
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##bossNextPatternSearch", "Search name or stable pattern ID...",
		m_NextPatternSearch.data(), m_NextPatternSearch.size());
	const std::string Query = m_NextPatternSearch.data();
	CValtanPatternAuditionService& Service = CValtanPatternAuditionService::Get();
	std::string SelectionStatus;
	LostArk::Shared::GameplayDataRevision ExpectedRevision{};
	/* Popup rendering must remain memory-only. The exact typed-source/Sound
	   admission runs once when Queue_NextServerPattern submits the command. */
	const bool_t bPlaybackAdmitted = Observe_ServerActivePatternRevision(
		ExpectedRevision, SelectionStatus);
	ImGui::BeginDisabled(!Can_MutateValtanView(m_eGraphAdmission) ||
		!m_bNextPatternInventoryReady ||
		!bPlaybackAdmitted ||
		!Service.Can_QueueNextPattern(
			BOSS_PLACEMENT_ID, ExpectedRevision, SelectionStatus));
	if (ImGui::BeginChild("##bossNextPatternChoices", ImVec2(0.f, 330.f), true))
	{
		for (const std::string& PatternId : m_NextPatternIds)
		{
			const VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(PatternId);
			if (nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
				(!Contains_CaseInsensitive(PatternId, Query) &&
				 !Contains_CaseInsensitive(pPattern->strDisplayName, Query)))
				continue;
			const bool_t bCompatibilityManual =
				"VALTAN_TRASH_CATCH_SUCCESS" == PatternId ||
				"VALTAN_TRASH_CATCH_FAIL" == PatternId ||
				"VALTAN_TRASH_CATCH_IF" == PatternId;
			const std::string Label = (pPattern->strDisplayName.empty() ?
				PatternId : pPattern->strDisplayName) +
				(bCompatibilityManual ? " [compatibility manual]" : "");
			ImGui::PushID(PatternId.c_str());
			if (ImGui::Selectable(Label.c_str(), false))
			{
				/* Selecting Next owns this decision even if the Server rejects it. */
				m_bRepeat = false;
				m_strRepeatPatternId.clear();
				(void)Queue_NextServerPattern(
					PatternId, m_strNextPatternStatus);
				ImGui::CloseCurrentPopup();
			}
			ImGui::TextDisabled("%s", PatternId.c_str());
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();
	ImGui::EndPopup();
}

void Client::CBossTool::Render_FlowGraphEditor()
{
	const VALTAN_PATTERN_FLOW_DEFINITION* const pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow)
	{
		ImGui::TextDisabled("No saved Pattern Flow is loaded.");
		return;
	}
	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const bool_t bEditingLocked =
		!Can_MutateValtanView(m_eGraphAdmission) ||
		CValtanPatternFlowService::Get().Has_PendingStart() ||
		CValtanPatternAuditionService::Get().Has_PlaybackOwnership() ||
		Is_ServerArenaPresetPending();
	ImGui::Text("Pattern Flow  |  %zu Patterns", pFlow->Nodes.size());
	ImGui::SameLine();
	ImGui::BeginDisabled(bEditingLocked);
	if (ImGui::Button("Add From Pattern Slot..."))
		ImGui::OpenPopup("##addBossFlowGraphNode");
	ImGui::EndDisabled();
	/* Inserting a node replaces the document draft.  Do not dereference the
	   pre-popup Flow pointer again in the same frame. */
	if (Render_AddPatternNodePopup())
		return;
	if (CValtanPatternFlowDocument::Has_LegacyLinearProjection(*pFlow))
	{
		ImGui::TextColored(
			ImVec4(0.38f, 0.88f, 0.58f, 1.f),
			"Ready for Server playback");
	}
	else
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.72f, 0.25f, 1.f),
			"Draft has a custom route");
		ImGui::TextDisabled(
			"Save is available. Choose one Next Pattern per row before Server playback.");
	}

	if (ImGui::BeginChild(
		"##bossPatternFlowGraphCanvas", ImVec2(0.f, 0.f), true))
	{
		if (ImGui::BeginTable(
				"##bossPatternRouteTable",
				4,
				ImGuiTableFlags_BordersInnerH |
				ImGuiTableFlags_BordersInnerV |
				ImGuiTableFlags_Resizable |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_SizingStretchProp |
				ImGuiTableFlags_ScrollY,
				ImVec2(0.f, 0.f)))
		{
			ImGui::TableSetupColumn(
				"#", ImGuiTableColumnFlags_WidthFixed, 34.f);
			ImGui::TableSetupColumn(
				"Pattern", ImGuiTableColumnFlags_WidthStretch, 1.35f);
			ImGui::TableSetupColumn(
				"Next Pattern", ImGuiTableColumnFlags_WidthStretch, 1.f);
			ImGui::TableSetupColumn(
				"Wait", ImGuiTableColumnFlags_WidthFixed, 76.f);
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			for (std::size_t index = 0u; index < pFlow->Nodes.size(); ++index)
			{
				const VALTAN_PATTERN_FLOW_NODE& Node = pFlow->Nodes[index];
				const VALTAN_PATTERN_VIEW* const pPattern =
					Find_AuditionPattern(Node.strPatternId);
				const std::string Name = nullptr != pPattern &&
					!pPattern->strDisplayName.empty() ?
						pPattern->strDisplayName : Node.strPatternId;
				const bool_t bSelected =
					m_strSelectedFlowSlotId == Node.strNodeId &&
					m_strSelectedFlowEdgeId.empty();
				const bool_t bEntry = pFlow->strEntryNodeId == Node.strNodeId;
				const bool_t bLive =
					Playback.strFlowRevision ==
						m_FlowDocument.Get_SourceRevision() &&
					Playback.strCurrentSlotId == Node.strNodeId;
				const bool_t bLinkSource =
					m_strFlowLinkSourceNodeId == Node.strNodeId;
				const auto Outgoing = std::find_if(
					pFlow->Edges.begin(), pFlow->Edges.end(),
					[&Node](const VALTAN_PATTERN_FLOW_EDGE& Edge)
					{
						return Edge.strFromNodeId == Node.strNodeId;
					});

				ImGui::PushID(Node.strNodeId.c_str());
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%02zu", index + 1u);

				ImGui::TableSetColumnIndex(1);
				std::string PatternLabel = Name;
				if (bEntry)
					PatternLabel += "  [First]";
				if (bLive)
					PatternLabel += "  [Playing]";
				if (bLinkSource)
					PatternLabel += "  [Choose Next]";
				if (ImGui::Selectable(
						PatternLabel.c_str(), bSelected,
						ImGuiSelectableFlags_None))
				{
					m_strSelectedFlowSlotId = Node.strNodeId;
					m_strSelectedFlowEdgeId.clear();
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(
						"Node: %s\nPattern: %s\nTimeout: %u ms",
						Node.strNodeId.c_str(), Node.strPatternId.c_str(),
						Node.iWatchdogMs);
				}

				ImGui::TableSetColumnIndex(2);
				if (pFlow->Edges.end() == Outgoing)
				{
					ImGui::TextDisabled("End");
				}
				else
				{
					const auto Target = std::find_if(
						pFlow->Nodes.begin(), pFlow->Nodes.end(),
						[&Outgoing](const VALTAN_PATTERN_FLOW_NODE& Candidate)
						{
							return Candidate.strNodeId == Outgoing->strToNodeId;
						});
					const VALTAN_PATTERN_VIEW* const pTargetPattern =
						pFlow->Nodes.end() == Target ? nullptr :
							Find_AuditionPattern(Target->strPatternId);
					std::string NextName = pFlow->Nodes.end() == Target ?
						std::string("Missing Pattern") :
						(nullptr != pTargetPattern &&
						 !pTargetPattern->strDisplayName.empty() ?
							pTargetPattern->strDisplayName : Target->strPatternId);
					if (Outgoing->iMaxTraversals.has_value())
						NextName += "  [Repeat]";
					const bool_t bEdgeSelected =
						m_strSelectedFlowEdgeId == Outgoing->strEdgeId;
					ImGui::PushID(Outgoing->strEdgeId.c_str());
					if (ImGui::Selectable(
							NextName.c_str(), bEdgeSelected,
							ImGuiSelectableFlags_None))
					{
						m_strSelectedFlowEdgeId = Outgoing->strEdgeId;
						m_strSelectedFlowSlotId = Node.strNodeId;
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip(
							"Edge: %s\nFrom: %s\nTo: %s",
							Outgoing->strEdgeId.c_str(),
							Outgoing->strFromNodeId.c_str(),
							Outgoing->strToNodeId.c_str());
					}
					ImGui::PopID();

					ImGui::TableSetColumnIndex(3);
					char_t WaitLabel[64]{};
					std::snprintf(
						WaitLabel, sizeof(WaitLabel), "%u ms##wait",
						Outgoing->iPursuitMs);
					ImGui::PushID(Outgoing->strEdgeId.c_str());
					if (ImGui::Selectable(
							WaitLabel, bEdgeSelected,
							ImGuiSelectableFlags_None))
					{
						m_strSelectedFlowEdgeId = Outgoing->strEdgeId;
						m_strSelectedFlowSlotId = Node.strNodeId;
					}
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Wait before the next Pattern starts.");
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
}

bool_t Client::CBossTool::Render_AddPatternNodePopup()
{
	if (!ImGui::BeginPopup("##addBossFlowGraphNode"))
		return false;
	bool_t bDocumentMutated = false;
	const bool_t bEditingLocked =
		!Can_MutateValtanView(m_eGraphAdmission) ||
		CValtanPatternFlowService::Get().Has_PendingStart() ||
		CValtanPatternAuditionService::Get().Has_PlaybackOwnership() ||
		Is_ServerArenaPresetPending();
	ImGui::BeginDisabled(bEditingLocked);
	ImGui::TextUnformatted("Insert a Pattern after the selected Pattern");
	ImGui::SetNextItemWidth(360.f);
	ImGui::InputTextWithHint(
		"##flowGraphPatternSearch", "Search Pattern...",
		m_FlowPatternSearch.data(), m_FlowPatternSearch.size());
	const std::string Query = m_FlowPatternSearch.data();
	const std::vector<std::string> AdmittedIds = Build_AdmittedPatternIds();
	if (ImGui::BeginChild(
		"##flowGraphPatternChoices", ImVec2(420.f, 330.f), true))
	{
		const auto RenderIds = [this, &Query, &AdmittedIds,
			&bDocumentMutated](
			const std::vector<std::string>& PatternIds)
		{
			for (const std::string& PatternId : PatternIds)
			{
				if (bDocumentMutated)
					break;
				const VALTAN_PATTERN_VIEW* const pPattern =
					Find_AuditionPattern(PatternId);
				if (nullptr == pPattern ||
					(!Contains_CaseInsensitive(pPattern->strPatternId, Query) &&
					 !Contains_CaseInsensitive(pPattern->strDisplayName, Query)))
				{
					continue;
				}
				ImGui::PushID(PatternId.c_str());
				const char_t* const pLabel = pPattern->strDisplayName.empty() ?
					pPattern->strPatternId.c_str() :
					pPattern->strDisplayName.c_str();
				if (ImGui::Selectable(pLabel, false))
				{
					std::string NodeId;
					std::string Status;
					if (m_FlowDocument.Insert_Node_After(
							m_strSelectedFlowSlotId, PatternId,
							AdmittedIds, NodeId, Status))
					{
						m_strSelectedFlowSlotId = NodeId;
						m_strSelectedFlowEdgeId.clear();
						bDocumentMutated = true;
						ImGui::CloseCurrentPopup();
					}
					m_strFlowStatus = std::move(Status);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s",
						CValtanPatternTree::Build_PatternIdentitySummary(
							*pPattern).c_str());
				ImGui::PopID();
			}
		};
		ImGui::SeparatorText("CORE SERVER PATTERNS");
		RenderIds(m_AuditionInventory.CorePatternIds);
		if (!bDocumentMutated)
		{
			ImGui::SeparatorText("ANIMATOR PATTERNS");
			RenderIds(m_AuditionInventory.AnimatorPatternIds);
		}
		if (!bDocumentMutated &&
			!m_AuditionInventory.DerivedPatternIds.empty())
		{
			ImGui::SeparatorText("DERIVED SERVER PATTERNS");
			RenderIds(m_AuditionInventory.DerivedPatternIds);
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();
	ImGui::EndPopup();
	return bDocumentMutated;
}

void Client::CBossTool::Render_FlowSlotList()
{
	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const bool_t bEditingLocked =
		!Can_MutateValtanView(m_eGraphAdmission) ||
		CValtanPatternFlowService::Get().Has_PendingStart() ||
		CValtanPatternAuditionService::Get().Has_PlaybackOwnership() ||
		Is_ServerArenaPresetPending();
	ImGui::TextUnformatted("Ordered Slots");
	ImGui::SameLine();
	ImGui::BeginDisabled(bEditingLocked);
	if (ImGui::Button("Add From Pattern Slot..."))
		ImGui::OpenPopup("##addBossFlowPattern");
	ImGui::EndDisabled();
	Render_AddPatternPopup();

	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow)
	{
		ImGui::TextDisabled("No admitted Flow is loaded.");
		return;
	}

	if (ImGui::BeginChild(
			"##bossPatternFlowSlots", ImVec2(0.f, -42.f), true))
	{
		for (size_t i = 0u; i < pFlow->Slots.size(); ++i)
		{
			const VALTAN_PATTERN_FLOW_SLOT& Slot = pFlow->Slots[i];
			const VALTAN_PATTERN_VIEW* pPattern =
				Find_AuditionPattern(Slot.strPatternId);
			const std::string Name = nullptr != pPattern &&
				!pPattern->strDisplayName.empty() ?
					pPattern->strDisplayName : Slot.strPatternId;
			char_t Label[512]{};
			const bool_t bLive =
				Playback.strFlowRevision ==
					m_FlowDocument.Get_SourceRevision() &&
				Playback.strCurrentSlotId == Slot.strSlotId;
			std::snprintf(
				Label, sizeof(Label), "%02zu  %s%s",
				i + 1u, Name.c_str(),
				bLive ? "  [LIVE]" : "");
			ImGui::PushID(Slot.strSlotId.c_str());
			if (ImGui::Selectable(
					Label, m_strSelectedFlowSlotId == Slot.strSlotId))
			{
				m_strSelectedFlowSlotId = Slot.strSlotId;
			}
			if (ImGui::IsItemHovered())
			{
				if (nullptr != pPattern)
					ImGui::SetTooltip("Slot: %s\n%s", Slot.strSlotId.c_str(),
						CValtanPatternTree::Build_PatternIdentitySummary(*pPattern).c_str());
				else
					ImGui::SetTooltip(
						"Slot: %s\nPattern ID: %s (outside shared inventory)", Slot.strSlotId.c_str(),
						Slot.strPatternId.c_str());
			}
			ImGui::PopID();
		}
		if (pFlow->Slots.empty())
			ImGui::TextDisabled("Add a Pattern from the shared All Effects inventory.");
	}
	ImGui::EndChild();

	const VALTAN_PATTERN_FLOW_SLOT* pSelected = Find_SelectedFlowSlot();
	const auto SelectedAt = nullptr == pSelected ? pFlow->Slots.end() :
		std::find_if(
			pFlow->Slots.begin(), pFlow->Slots.end(),
			[this](const VALTAN_PATTERN_FLOW_SLOT& Slot)
			{
				return Slot.strSlotId == m_strSelectedFlowSlotId;
			});
	const bool_t bHasSelection = pFlow->Slots.end() != SelectedAt;
	const bool_t bSelectedIsEntry = nullptr != pSelected &&
		Is_OptionalEntryPatternId(pSelected->strPatternId);
	const bool_t bFirstIsEntry = !pFlow->Slots.empty() &&
		Is_OptionalEntryPatternId(pFlow->Slots.front().strPatternId);
	const bool_t bWouldCrossEntry = bHasSelection && bFirstIsEntry &&
		SelectedAt == std::next(pFlow->Slots.begin());
	const bool_t bCanMoveUp = bHasSelection &&
		SelectedAt != pFlow->Slots.begin() && !bWouldCrossEntry;
	const bool_t bCanMoveDown = bHasSelection &&
		std::next(SelectedAt) != pFlow->Slots.end() && !bSelectedIsEntry;
	ImGui::BeginDisabled(bEditingLocked || !bCanMoveUp);
	if (ImGui::Button("Up"))
	{
		std::string Status;
		if (!m_FlowDocument.Move_Slot(
				m_strSelectedFlowSlotId, -1,
				Build_AdmittedPatternIds(), Status))
			m_strFlowStatus = Status;
		else
			m_strFlowStatus = Status;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(bEditingLocked || !bCanMoveDown);
	if (ImGui::Button("Down"))
	{
		std::string Status;
		if (!m_FlowDocument.Move_Slot(
				m_strSelectedFlowSlotId, 1,
				Build_AdmittedPatternIds(), Status))
			m_strFlowStatus = Status;
		else
			m_strFlowStatus = Status;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(bEditingLocked || !bHasSelection);
	if (ImGui::Button("Discard Selected"))
	{
		const size_t OldIndex = static_cast<size_t>(
			std::distance(pFlow->Slots.begin(), SelectedAt));
		const std::string RemovedSlotId = m_strSelectedFlowSlotId;
		std::string Status;
		if (m_FlowDocument.Remove_Slot(RemovedSlotId, Status))
		{
			const VALTAN_PATTERN_FLOW_DEFINITION* pUpdated =
				m_FlowDocument.Get_DefaultFlow();
			if (nullptr == pUpdated || pUpdated->Slots.empty())
				m_strSelectedFlowSlotId.clear();
			else
				m_strSelectedFlowSlotId = pUpdated->Slots[
					std::min(OldIndex, pUpdated->Slots.size() - 1u)].strSlotId;
		}
		m_strFlowStatus = Status;
	}
	ImGui::EndDisabled();
}

void Client::CBossTool::Render_AddPatternPopup()
{
	if (!ImGui::BeginPopup("##addBossFlowPattern"))
		return;
	const bool_t bEditingLocked =
		!Can_MutateValtanView(m_eGraphAdmission) ||
		CValtanPatternFlowService::Get().Has_PendingStart() ||
		CValtanPatternAuditionService::Get().Has_PlaybackOwnership() ||
		Is_ServerArenaPresetPending();
	ImGui::BeginDisabled(bEditingLocked);
	ImGui::TextUnformatted("Add From Pattern Slot");
	ImGui::SetNextItemWidth(360.f);
	ImGui::InputTextWithHint(
		"##flowPatternSearch", "Search pattern...",
		m_FlowPatternSearch.data(), m_FlowPatternSearch.size());
	const std::string Query = m_FlowPatternSearch.data();
	const std::vector<std::string> AdmittedIds = Build_AdmittedPatternIds();
	if (ImGui::BeginChild(
			"##flowPatternChoices", ImVec2(420.f, 330.f), true))
	{
		const auto RenderIds = [this, &Query, &AdmittedIds](
			const std::vector<std::string>& PatternIds)
		{
			for (const std::string& PatternId : PatternIds)
			{
				const VALTAN_PATTERN_VIEW* pPattern =
					Find_AuditionPattern(PatternId);
				if (nullptr == pPattern ||
					(!Contains_CaseInsensitive(pPattern->strPatternId, Query) &&
					 !Contains_CaseInsensitive(pPattern->strDisplayName, Query)))
				{
					continue;
				}
				ImGui::PushID(PatternId.c_str());
				const char_t* pLabel = pPattern->strDisplayName.empty() ?
					pPattern->strPatternId.c_str() :
					pPattern->strDisplayName.c_str();
				if (ImGui::Selectable(pLabel, false))
				{
					std::string SlotId;
					std::string Status;
					if (m_FlowDocument.Add_Slot(
							PatternId, AdmittedIds, SlotId, Status))
					{
						m_strSelectedFlowSlotId = SlotId;
						ImGui::CloseCurrentPopup();
					}
					m_strFlowStatus = Status;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s",
						CValtanPatternTree::Build_PatternIdentitySummary(*pPattern).c_str());
				ImGui::PopID();
			}
		};
		ImGui::SeparatorText((std::string("CORE SERVER PATTERNS (") +
			std::to_string(m_AuditionInventory.CorePatternIds.size()) + ")").c_str());
		RenderIds(m_AuditionInventory.CorePatternIds);
		ImGui::SeparatorText((std::string("ANIMATOR PATTERNS (") +
			std::to_string(m_AuditionInventory.AnimatorPatternIds.size()) + ")").c_str());
		RenderIds(m_AuditionInventory.AnimatorPatternIds);
		if (!m_AuditionInventory.DerivedPatternIds.empty())
		{
			ImGui::SeparatorText((std::string("DERIVED SERVER PATTERNS (") +
				std::to_string(m_AuditionInventory.DerivedPatternIds.size()) + ")").c_str());
			RenderIds(m_AuditionInventory.DerivedPatternIds);
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();
	ImGui::EndPopup();
}

void Client::CBossTool::Render_FlowSelectedSlot()
{
	const VALTAN_PATTERN_FLOW_DEFINITION* const pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow)
		return;
	const VALTAN_PATTERN_FLOW_NODE* const pNode = Find_SelectedFlowNode();
	const VALTAN_PATTERN_FLOW_EDGE* const pEdge = Find_SelectedFlowEdge();
	const VALTAN_PATTERN_VIEW* const pPattern = nullptr == pNode ? nullptr :
		Find_AuditionPattern(pNode->strPatternId);
	CValtanPatternFlowService& FlowService = CValtanPatternFlowService::Get();
	const bool_t bEditingLocked =
		!Can_MutateValtanView(m_eGraphAdmission) ||
		FlowService.Has_PendingStart() ||
		CValtanPatternAuditionService::Get().Has_PlaybackOwnership() ||
		Is_ServerArenaPresetPending();
	const std::vector<std::string> AdmittedIds = Build_AdmittedPatternIds();

	if (nullptr == pNode || nullptr == pPattern)
	{
		ImGui::TextUnformatted("Select a Pattern on the left.");
	}
	else
	{
		const auto NodeAt = std::find_if(
			pFlow->Nodes.begin(), pFlow->Nodes.end(),
			[this](const VALTAN_PATTERN_FLOW_NODE& Node)
			{ return Node.strNodeId == m_strSelectedFlowSlotId; });
		const size_t Ordinal = pFlow->Nodes.end() == NodeAt ? 0u :
			static_cast<size_t>(std::distance(pFlow->Nodes.begin(), NodeAt)) + 1u;
		ImGui::Text(
			"%02zu  %s", Ordinal,
			pPattern->strDisplayName.empty() ?
				pPattern->strPatternId.c_str() :
				pPattern->strDisplayName.c_str());
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(
				"Node: %s\nPattern: %s",
				pNode->strNodeId.c_str(), pNode->strPatternId.c_str());
		ImGui::TextWrapped("%s",
			CValtanPatternTree::Build_PatternIdentitySummary(*pPattern).c_str());

		ImGui::SeparatorText("Pattern Order");
		ImGui::BeginDisabled(bEditingLocked);
		const bool_t bLinearOrder =
			CValtanPatternFlowDocument::Has_LegacyLinearProjection(*pFlow);
		const auto LinearSlotAt = !bLinearOrder ? pFlow->Slots.end() :
			std::find_if(
				pFlow->Slots.begin(), pFlow->Slots.end(),
				[this](const VALTAN_PATTERN_FLOW_SLOT& Slot)
				{ return Slot.strSlotId == m_strSelectedFlowSlotId; });
		const bool_t bHasLinearSelection =
			bLinearOrder && pFlow->Slots.end() != LinearSlotAt;
		const bool_t bSelectedIsEntrance = bHasLinearSelection &&
			"VALTAN_ENTRANCE_CINEMATIC" == LinearSlotAt->strPatternId;
		const bool_t bFirstIsEntrance = bLinearOrder && !pFlow->Slots.empty() &&
			"VALTAN_ENTRANCE_CINEMATIC" == pFlow->Slots.front().strPatternId;
		const bool_t bWouldCrossEntrance = bHasLinearSelection &&
			bFirstIsEntrance && LinearSlotAt == std::next(pFlow->Slots.begin());
		const bool_t bCanMoveUp = bHasLinearSelection &&
			LinearSlotAt != pFlow->Slots.begin() && !bWouldCrossEntrance;
		const bool_t bCanMoveDown = bHasLinearSelection &&
			std::next(LinearSlotAt) != pFlow->Slots.end() &&
			!bSelectedIsEntrance;
		bool_t bOrderMutated = false;
		ImGui::BeginDisabled(!bCanMoveUp);
		if (ImGui::Button("Up##FlowGraphOrder"))
		{
			std::string Status;
			bOrderMutated = m_FlowDocument.Move_Slot(
				pNode->strNodeId, -1, AdmittedIds, Status);
			m_strFlowStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
			!bLinearOrder)
		{
			ImGui::SetTooltip(
				"Up/Down is available when this route is one saved-order-compatible linear chain.");
		}
		if (bOrderMutated)
		{
			m_strSelectedFlowEdgeId.clear();
			ImGui::EndDisabled();
			return;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(!bCanMoveDown);
		if (ImGui::Button("Down##FlowGraphOrder"))
		{
			std::string Status;
			bOrderMutated = m_FlowDocument.Move_Slot(
				pNode->strNodeId, 1, AdmittedIds, Status);
			m_strFlowStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
			!bLinearOrder)
		{
			ImGui::SetTooltip(
				"Up/Down is available when this route is one saved-order-compatible linear chain.");
		}
		if (bOrderMutated)
		{
			m_strSelectedFlowEdgeId.clear();
			ImGui::EndDisabled();
			return;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(pFlow->strEntryNodeId == pNode->strNodeId);
		if (ImGui::Button("Make First Pattern"))
		{
			std::string Status;
			if (m_FlowDocument.Set_EntryNode(
					pNode->strNodeId, AdmittedIds, Status))
			{
				m_strFlowStatus = std::move(Status);
				ImGui::EndDisabled();
				ImGui::EndDisabled();
				return;
			}
			m_strFlowStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(pFlow->Nodes.size() <= 1u);
		if (ImGui::Button("Discard Selected"))
		{
			const std::string RemovedNodeId = pNode->strNodeId;
			std::string Status;
			if (m_FlowDocument.Remove_Node(
					RemovedNodeId, AdmittedIds, Status))
			{
				const VALTAN_PATTERN_FLOW_DEFINITION* const pUpdated =
					m_FlowDocument.Get_DefaultFlow();
				m_strSelectedFlowSlotId = nullptr == pUpdated ?
					std::string{} : pUpdated->strEntryNodeId;
				m_strSelectedFlowEdgeId.clear();
				if (m_strFlowLinkSourceNodeId == RemovedNodeId)
					m_strFlowLinkSourceNodeId.clear();
				m_strFlowStatus = std::move(Status);
				ImGui::EndDisabled();
				ImGui::EndDisabled();
				return;
			}
			m_strFlowStatus = std::move(Status);
		}
		ImGui::EndDisabled();

		bool_t bWatchdogEnabled = 0u != pNode->iWatchdogMs;
		if (ImGui::Checkbox("Pattern Timeout", &bWatchdogEnabled))
		{
			std::string Status;
			const std::uint32_t Watchdog = bWatchdogEnabled ?
				CValtanPatternFlowDocument::MIN_NODE_WATCHDOG_MS : 0u;
			if (m_FlowDocument.Set_NodeWatchdogMs(
					pNode->strNodeId, Watchdog, AdmittedIds, Status))
			{
				m_strFlowStatus = std::move(Status);
				ImGui::EndDisabled();
				return;
			}
			m_strFlowStatus = std::move(Status);
		}
		if (0u != pNode->iWatchdogMs)
		{
			int32_t WatchdogMs = static_cast<int32_t>(pNode->iWatchdogMs);
			ImGui::SetNextItemWidth(230.f);
			if (ImGui::SliderInt(
				"Timeout (ms)", &WatchdogMs,
				static_cast<int32_t>(
					CValtanPatternFlowDocument::MIN_NODE_WATCHDOG_MS),
				static_cast<int32_t>(
					CValtanPatternFlowDocument::MAX_NODE_WATCHDOG_MS),
				"%d ms", ImGuiSliderFlags_AlwaysClamp))
			{
				std::string Status;
				if (m_FlowDocument.Set_NodeWatchdogMs(
						pNode->strNodeId,
						static_cast<std::uint32_t>(WatchdogMs),
						AdmittedIds, Status))
				{
					m_strFlowStatus = std::move(Status);
					ImGui::EndDisabled();
					return;
				}
				m_strFlowStatus = std::move(Status);
			}
		}

		if (m_strFlowLinkSourceNodeId.empty())
		{
			if (ImGui::Button("Choose Next Pattern"))
			{
				m_strFlowLinkSourceNodeId = pNode->strNodeId;
				m_strSelectedFlowEdgeId.clear();
				m_strFlowStatus =
					"Choose the destination Pattern on the left, then confirm it here.";
			}
		}
		else
		{
			const auto SourceNode = std::find_if(
				pFlow->Nodes.begin(), pFlow->Nodes.end(),
				[this](const VALTAN_PATTERN_FLOW_NODE& Candidate)
				{
					return Candidate.strNodeId == m_strFlowLinkSourceNodeId;
				});
			const VALTAN_PATTERN_VIEW* const pSourcePattern =
				pFlow->Nodes.end() == SourceNode ? nullptr :
					Find_AuditionPattern(SourceNode->strPatternId);
			const std::string SourceName =
				pFlow->Nodes.end() == SourceNode ? std::string("Missing Pattern") :
				(nullptr != pSourcePattern &&
				 !pSourcePattern->strDisplayName.empty() ?
					pSourcePattern->strDisplayName : SourceNode->strPatternId);
			ImGui::TextWrapped("From: %s", SourceName.c_str());
			ImGui::TextWrapped("To: %s",
				pPattern->strDisplayName.empty() ?
					pPattern->strPatternId.c_str() :
					pPattern->strDisplayName.c_str());
			int32_t Traversals = static_cast<int32_t>(
				m_iFlowLinkMaximumTraversals);
			ImGui::SetNextItemWidth(230.f);
			if (ImGui::SliderInt(
				"Repeat returns", &Traversals, 1,
				static_cast<int32_t>(
					CValtanPatternFlowDocument::MAX_EDGE_TRAVERSALS),
				"%d", ImGuiSliderFlags_AlwaysClamp))
			{
				m_iFlowLinkMaximumTraversals =
					static_cast<std::uint32_t>(Traversals);
			}
			ImGui::TextDisabled(
				"After reaching the destination once, repeat this return a limited number of times.");
			if (ImGui::Button("Set Next to Selected"))
			{
				std::string EdgeId;
				std::string Status;
				if (m_FlowDocument.Connect_CompletedEdge(
						m_strFlowLinkSourceNodeId, pNode->strNodeId,
						pFlow->iDefaultPursuitMs,
						m_iFlowLinkMaximumTraversals,
						AdmittedIds, EdgeId, Status))
				{
					m_strSelectedFlowEdgeId = EdgeId;
					m_strFlowLinkSourceNodeId.clear();
					m_strFlowStatus = std::move(Status);
					ImGui::EndDisabled();
					return;
				}
				m_strFlowStatus = std::move(Status);
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_strFlowLinkSourceNodeId.clear();
				m_strFlowStatus = "Cancelled the pending Flow link.";
			}
		}
		ImGui::EndDisabled();
	}

	if (nullptr != pEdge)
	{
		const auto TargetNode = std::find_if(
			pFlow->Nodes.begin(), pFlow->Nodes.end(),
			[pEdge](const VALTAN_PATTERN_FLOW_NODE& Candidate)
			{
				return Candidate.strNodeId == pEdge->strToNodeId;
			});
		const VALTAN_PATTERN_VIEW* const pTargetPattern =
			pFlow->Nodes.end() == TargetNode ? nullptr :
				Find_AuditionPattern(TargetNode->strPatternId);
		const std::string TargetName =
			pFlow->Nodes.end() == TargetNode ? std::string("Missing Pattern") :
			(nullptr != pTargetPattern &&
			 !pTargetPattern->strDisplayName.empty() ?
				pTargetPattern->strDisplayName : TargetNode->strPatternId);
		ImGui::SeparatorText("Next Pattern");
		ImGui::TextWrapped("%s", TargetName.c_str());
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Edge: %s\nFrom: %s\nTo: %s",
				pEdge->strEdgeId.c_str(), pEdge->strFromNodeId.c_str(),
				pEdge->strToNodeId.c_str());
		}
		ImGui::BeginDisabled(bEditingLocked);
		int32_t EdgePursuitMs = static_cast<int32_t>(pEdge->iPursuitMs);
		ImGui::SetNextItemWidth(230.f);
		if (ImGui::SliderInt(
			"Wait before next (ms)", &EdgePursuitMs, 100, 10000,
			"%d ms", ImGuiSliderFlags_AlwaysClamp))
		{
			std::string Status;
			if (m_FlowDocument.Set_EdgePursuitMs(
					pEdge->strEdgeId,
					static_cast<std::uint32_t>(EdgePursuitMs),
					AdmittedIds, Status))
			{
				m_strFlowStatus = std::move(Status);
				ImGui::EndDisabled();
				return;
			}
			m_strFlowStatus = std::move(Status);
		}
		if (pEdge->iMaxTraversals.has_value())
		{
			int32_t Traversals = static_cast<int32_t>(
				*pEdge->iMaxTraversals);
			ImGui::SetNextItemWidth(230.f);
			if (ImGui::SliderInt(
				"Repeat returns", &Traversals, 1,
				static_cast<int32_t>(
					CValtanPatternFlowDocument::MAX_EDGE_TRAVERSALS),
				"%d", ImGuiSliderFlags_AlwaysClamp))
			{
				std::string Status;
				if (m_FlowDocument.Set_EdgeMaxTraversals(
						pEdge->strEdgeId,
						static_cast<std::uint32_t>(Traversals),
						AdmittedIds, Status))
				{
					m_strFlowStatus = std::move(Status);
					ImGui::EndDisabled();
					return;
				}
				m_strFlowStatus = std::move(Status);
			}
		}
		else
		{
			ImGui::TextDisabled(
				"This is the normal next Pattern. A repeated return also exposes a repeat count.");
		}
		if (ImGui::Button("Remove Next Pattern"))
		{
			std::string Status;
			if (m_FlowDocument.Remove_Edge(
					pEdge->strEdgeId, AdmittedIds, Status))
			{
				m_strSelectedFlowEdgeId.clear();
				m_strFlowStatus = std::move(Status);
				ImGui::EndDisabled();
				return;
			}
			m_strFlowStatus = std::move(Status);
		}
		ImGui::EndDisabled();
	}

	ImGui::SeparatorText("Run Limit");
	int32_t MaxTransitions = static_cast<int32_t>(
		pFlow->iMaxTransitionsPerRun);
	ImGui::BeginDisabled(bEditingLocked);
	ImGui::SetNextItemWidth(230.f);
	if (ImGui::SliderInt(
			"Maximum Pattern changes", &MaxTransitions,
		static_cast<int32_t>(
			CValtanPatternFlowDocument::MIN_TRANSITIONS_PER_RUN),
		static_cast<int32_t>(
			CValtanPatternFlowDocument::MAX_TRANSITIONS_PER_RUN),
		"%d", ImGuiSliderFlags_AlwaysClamp))
	{
		std::string Status;
		if (m_FlowDocument.Set_MaxTransitionsPerRun(
				static_cast<std::uint32_t>(MaxTransitions),
				AdmittedIds, Status))
		{
			m_strFlowStatus = std::move(Status);
			ImGui::EndDisabled();
			return;
		}
		m_strFlowStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Stops a repeated route after this many Pattern changes.");

	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback = FlowService.Get_Snapshot();
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Isolated =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t bRuntimeReady = CNetworkManager::Get().Is_Connected() &&
		Boss.isValid && Player.isValid && 0u != Player.iCurrentHp &&
		Player.isCombatReady;
	const bool_t bCanPreview = Can_MutateValtanView(m_eGraphAdmission) &&
		nullptr != pNode && nullptr != pPattern &&
		bRuntimeReady && !FlowService.Has_PlaybackOwnership() &&
		!CValtanPatternAuditionService::Get().Has_PlaybackOwnership();
	ImGui::BeginDisabled(!bCanPreview);
	if (ImGui::Button("Preview Isolated"))
		(void)Preview_SelectedFlowSlotIsolated();
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Uses the same Server single-pattern path as Play Selected.");

	ImGui::SeparatorText("Flow Playback");
	int32_t PursuitMs = static_cast<int32_t>(pFlow->iDefaultPursuitMs);
	ImGui::BeginDisabled(bEditingLocked);
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::SliderInt(
			"Inter-step pursuit (ms)", &PursuitMs,
			100, 10000, "%d ms", ImGuiSliderFlags_AlwaysClamp))
	{
		std::string Status;
		if (m_FlowDocument.Set_InterStepPursuitMs(
				static_cast<uint32_t>(PursuitMs), Status))
		{
			m_strFlowStatus = std::move(Status);
			ImGui::EndDisabled();
			return;
		}
		m_strFlowStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	const bool_t bSavedClean = m_FlowDocument.Is_Ready() &&
		!m_FlowDocument.Is_Dirty() &&
		!m_FlowDocument.Has_ExternalConflict();
	std::string DraftValidationStatus;
	const bool_t bDraftAdmitted = CValtanPatternFlowDocument::Validate(
		m_FlowDocument.Get_Draft(), Build_AdmittedPatternIds(),
		DraftValidationStatus);
	const bool_t bHasServerProjection =
		CValtanPatternFlowDocument::Has_LegacyLinearProjection(*pFlow);
	ImGui::BeginDisabled(
		!Playback.Is_InFlight() || 0u == Playback.iRoomFlowEpoch ||
		Playback.bStopAfterCurrentRequested);
	if (ImGui::Button("Stop After Current##flow"))
	{
		std::string Status;
		if (Can_MutateCanonicalGraph(Status))
			(void)CValtanPatternFlowService::Get().Stop_AfterCurrent(Status);
		m_strFlowStatus = Status;
	}
	ImGui::EndDisabled();
	if (Player.isValid && 0u == Player.iCurrentHp)
	{
		ImGui::SameLine();
		ImGui::BeginDisabled(nullptr == m_pCommandSink);
		if (ImGui::Button("Revive Player##flow"))
			(void)Request_RevivePlayer(m_strFlowStatus);
		ImGui::EndDisabled();
	}

	ImGui::TextWrapped("Tool: %s", m_strFlowStatus.c_str());
	ImGui::TextWrapped(
		"Server: %s | %s",
		Describe_ValtanPatternFlowState(Playback.eState),
		Playback.strStatus.empty() ?
			"No gameplay sequence is active. Restart Saved Flow (Fresh Arena) reloads the complete saved order, restores the arena, starts Pattern 01, then follows its saved Next Pattern and Wait values." :
			Playback.strStatus.c_str());
	if (!Playback.strFlowRevision.empty())
	{
		if (Playback.Is_InFlight() &&
			Playback.strFlowRevision != m_FlowDocument.Get_SourceRevision())
		{
			ImGui::TextWrapped(
				"The current run keeps its pinned revision. Restart Saved Flow (Fresh Arena) loads the latest complete saved gameplay order and restores the arena before Pattern 01.");
		}
	}
	const VALTAN_PATTERN_FLOW_START_COMMAND& StartCommand = FlowService.Get_PendingStart();
	if (!StartCommand.strStatus.empty())
		ImGui::TextWrapped("Start request: %s", StartCommand.strStatus.c_str());
	if (FLOW_PREVIEW_CONSUMER_ID == Isolated.strConsumerId &&
		VALTAN_PATTERN_AUDITION_STATE::IDLE != Isolated.eState &&
		!Playback.Is_InFlight())
	{
		ImGui::TextWrapped(
			"Isolated preview: %s", Isolated.strStatus.c_str());
	}
	if ((!bSavedClean || !bDraftAdmitted) && !Playback.Is_InFlight())
	{
		ImGui::TextDisabled(
			"Save a clean Flow to enable Restart. Isolated preview remains available.");
		if (!bDraftAdmitted)
			ImGui::TextWrapped(
				"Inventory validation: %s", DraftValidationStatus.c_str());
	}
	if (!bHasServerProjection && !Playback.Is_InFlight())
	{
		ImGui::TextDisabled(
			"This v2 graph can be saved, but Server Flow playback stays disabled until the graph runtime admits watchdogs/back-edges/per-edge pursuit.");
	}
}

void Client::CBossTool::Render_LiveSummary()
{
	const bool_t bConnected = CNetworkManager::Get().Is_Connected();
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	ImGui::Checkbox("Follow Live", &m_bFollowLive);
	if (!bConnected)
	{
		ImGui::TextUnformatted(
			"Live: Server disconnected. Start Debug Server and enter Valtan Arena.");
	}
	else if (!Boss.isValid)
	{
		ImGui::TextUnformatted(
			"Live: No replicated Valtan. Enter the Arena or spawn the encounter boss.");
	}
	else
	{
		const VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(Boss.strPatternId);
		const VALTAN_STAGE_VIEW* pStage = nullptr == pPattern ?
			nullptr : Find_LiveStage(*pPattern);
		const ENCOUNTER_PATTERN_REFERENCE* const pFallbackPattern =
			nullptr == pPattern ?
				Find_ProductFallbackPattern(Boss.strPatternId) : nullptr;
		const ENCOUNTER_STAGE_REFERENCE* const pFallbackStage =
			nullptr == pFallbackPattern ? nullptr :
				Find_ProductFallbackStage(*pFallbackPattern, Boss.strActionId);
		std::string PatternText = Boss.strPatternId.empty() ?
			"IDLE" :
			(nullptr != pPattern && !pPattern->strDisplayName.empty() ?
				pPattern->strDisplayName :
				(nullptr != pFallbackPattern &&
				 !pFallbackPattern->displayName.empty() ?
					pFallbackPattern->displayName : Boss.strPatternId));
		if (nullptr != pFallbackPattern)
		{
			PatternText += " [READ-ONLY PRODUCT FALLBACK]";
		}
		else if (!Boss.strPatternId.empty() &&
			!m_AuditionInventory.Contains(Boss.strPatternId))
		{
			PatternText += " [live only; outside All Effects list]";
		}
		std::string StageText = nullptr == pStage ?
			(nullptr != pFallbackStage ? pFallbackStage->stageId :
				(Boss.strActionId.empty() ? "IDLE" :
					"UNKNOWN ACTION " + Boss.strActionId)) :
			pStage->strStageId;

		std::string TimeText = "tick " + std::to_string(Boss.iServerTick);
		const CEncounterPatternReference* const pTimingReference =
			nullptr != pStage ? &m_EncounterReference :
				(nullptr != pFallbackStage ?
					&m_ProductFallbackEncounterReference : nullptr);
		const uint32_t iStageDurationMs = nullptr != pStage ?
			pStage->iDurationMs :
			(nullptr != pFallbackStage ? pFallbackStage->iDurationMs : 0u);
		if (nullptr != pTimingReference && pTimingReference->Is_Ready() &&
			0u != pTimingReference->Get_FixedTickHz() &&
			0u != iStageDurationMs)
		{
			const uint32_t iElapsedTicks = Boss.iServerTick >= Boss.iActionStartTick ?
				Boss.iServerTick - Boss.iActionStartTick : 0u;
			const f32_t fElapsedSeconds = static_cast<f32_t>(iElapsedTicks) /
				static_cast<f32_t>(pTimingReference->Get_FixedTickHz());
			char_t Buffer[96]{};
			std::snprintf(
				Buffer, sizeof(Buffer), "%.2f / %.2f s",
				fElapsedSeconds,
				static_cast<f32_t>(iStageDurationMs) / 1000.f);
			TimeText = Buffer;
		}

		const uint32_t iHealthBars = 0u == Boss.iMaximumHp ? 0u :
			static_cast<uint32_t>((
				static_cast<uint64_t>(Boss.iCurrentHp) *
				Boss.iMaximumHealthBars + Boss.iMaximumHp - 1u) /
				Boss.iMaximumHp);
		const char_t* pFreshness = "revision pending";
		if (Boss.PinnedDefinitionRevision.Is_Valid())
		{
			const bool_t bRevisionAvailable =
				CNetworkManager::Get().Is_PresentationRevisionAvailable(
					Boss.PinnedDefinitionRevision);
			pFreshness = !bRevisionAvailable ?
				"presentation UNAVAILABLE" :
				(m_bPresentationBaselineIntact ?
					"core presentation matches workspace" :
					"saved files changed; restart Server");
		}
		ImGui::TextWrapped(
			"Live: %s / %s  |  %s  |  Phase %u  |  HP %u bars  |  %s",
			PatternText.c_str(), StageText.c_str(), TimeText.c_str(),
			static_cast<uint32_t>(Boss.iPhase), iHealthBars, pFreshness);
		if (((nullptr != pPattern && !pPattern->strDisplayName.empty()) ||
			 (nullptr != pFallbackPattern &&
			  !pFallbackPattern->displayName.empty())) &&
			ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Pattern ID: %s", Boss.strPatternId.c_str());
		}
	}
}

void Client::CBossTool::Render_ActionBar()
{
	const CValtanPatternAuditionService& PatternService =
		CValtanPatternAuditionService::Get();
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Audition =
		PatternService.Get_Snapshot();
	const CValtanPatternFlowService& FlowService =
		CValtanPatternFlowService::Get();
	const bool_t bFlowOwnsPlayback =
		FlowService.Has_PlaybackOwnership();
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	const VALTAN_PATTERN_VIEW* pSelected =
		Find_AuditionPattern(m_strSelectedPatternId);
	const bool_t bCanPlay = Can_MutateValtanView(m_eGraphAdmission) &&
		nullptr != pSelected &&
		CNetworkManager::Get().Is_Connected() && Boss.isValid &&
		Player.isValid && 0u != Player.iCurrentHp && Player.isCombatReady &&
		!PatternService.Has_PlaybackOwnership() &&
		!bFlowOwnsPlayback;
	const bool_t bNextOwnsPlayback =
		PatternService.Get_NextSnapshot().Is_Live() ||
		PatternService.Has_PendingNextCommand();
	const bool_t bRestartablePatternOccurrence =
		VALTAN_PATTERN_AUDITION_STATE::ACTIVE == Audition.eState ||
		VALTAN_PATTERN_AUDITION_STATE::COMPLETED == Audition.eState;
	const bool_t bCanRestartActivePattern =
		Can_MutateValtanView(m_eGraphAdmission) && nullptr != pSelected &&
		m_strSelectedPatternId == Audition.strPatternId &&
		CNetworkManager::Get().Is_Connected() && Boss.isValid &&
		Player.isValid && 0u != Player.iCurrentHp && Player.isCombatReady &&
		!Is_ServerArenaPresetPending() && !bFlowOwnsPlayback &&
		!bNextOwnsPlayback && bRestartablePatternOccurrence &&
		CONSUMER_ID == Audition.strConsumerId &&
		BOSS_PLACEMENT_ID == Audition.strBossPlacementId;
	const bool_t bCanRetryRestart = CNetworkManager::Get().Is_Connected() &&
		VALTAN_PATTERN_AUDITION_STATE::RESTART_UNCONFIRMED ==
			Audition.eState &&
		CONSUMER_ID == Audition.strConsumerId &&
		BOSS_PLACEMENT_ID == Audition.strBossPlacementId;

	ImGui::TextDisabled("Pattern Playback:");
	ImGui::SameLine();
	ImGui::TextUnformatted(nullptr == pSelected ?
		"Select a pattern" :
		(pSelected->strDisplayName.empty() ?
			pSelected->strPatternId.c_str() :
			pSelected->strDisplayName.c_str()));
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanPlay);
	if (ImGui::Button("Play Selected Pattern (Keep Arena)"))
	{
#ifdef _DEBUG
		if (CMainApp* const pApp = CMainApp::Get_Active())
		{
			if (pApp->Debug_SelectCompletePlayPattern(
					m_strSelectedPatternId))
			{
				(void)pApp->Debug_CompletePlaySelected(m_strStatus);
			}
			else
			{
				m_strStatus =
					"Selected Boss pattern is not in the shared Server inventory.";
			}
		}
		else
			m_strStatus = "Complete Play workspace is unavailable.";
#else
		m_strStatus =
			"Complete Play is available only in a Debug authoring build.";
#endif
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip(
			"Boss-only reset through PLAY_PATTERN_ID. Plays only the selected Pattern from its first Stage; keeps current walls, floors, props, collision, and Nav; cancels only the replaced boss's combat objects; and does not run the saved Flow, Next Pattern, or inter-pattern Wait.");
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanRestartActivePattern);
	if (ImGui::Button("Restart Active Pattern (Keep Arena)"))
		(void)Restart_SelectedPattern();
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip(
			"Exact-occurrence restart through RESTART_PATTERN_ID. Restarts only this Tool's active or completed selected Pattern from its first Stage; keeps current walls, floors, props, collision, and Nav; cancels only the replaced boss's combat objects; and does not reload the saved Flow.");
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanRetryRestart);
	if (ImGui::Button("Retry Restart Verdict"))
	{
		std::string RetryStatus;
		LostArk::Shared::GameplayDataRevision ExpectedRevision{};
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT PinnedSoundReceipt;
		CValtanPatternSoundSourceReadAdmission SoundAdmission;
		if (Acquire_ServerPlaybackAdmission(
				ExpectedRevision, PinnedSoundReceipt,
				SoundAdmission, RetryStatus))
		{
			const VALTAN_PATTERN_AUDITION_SNAPSHOT& RetrySnapshot =
				CValtanPatternAuditionService::Get().Get_Snapshot();
			if (RetrySnapshot.PinnedDefinitionRevision.Is_Valid() &&
				RetrySnapshot.PinnedDefinitionRevision != ExpectedRevision)
			{
				RetryStatus =
					"Restart retry rejected because the active Pattern changed. Start a fresh Pattern request.";
			}
			else
			{
				(void)CValtanPatternAuditionService::Get().Retry_UnconfirmedRestart(
					PinnedSoundReceipt, RetryStatus);
			}
		}
		m_strStatus = std::move(RetryStatus);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!Can_MutateValtanView(m_eGraphAdmission) ||
		nullptr == pSelected ||
		bNextOwnsPlayback || bFlowOwnsPlayback);
	if (ImGui::Checkbox("Repeat", &m_bRepeat))
	{
		m_bReviveFeedbackPending = false;
		m_strActionFeedback.clear();
		if (!m_bRepeat)
			m_strRepeatPatternId.clear();
		else if (Audition.Is_InFlight() &&
			CONSUMER_ID == Audition.strConsumerId)
			m_strRepeatPatternId = Audition.strPatternId;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bRepeat && m_strRepeatPatternId.empty());
	if (ImGui::Button("Stop After Current"))
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
		m_bReviveFeedbackPending = false;
		m_strActionFeedback =
			"Repeat stopped. The current Server pattern will finish normally.";
	}
	ImGui::EndDisabled();
	if (Player.isValid && 0u == Player.iCurrentHp)
	{
		ImGui::SameLine();
		ImGui::BeginDisabled(nullptr == m_pCommandSink);
		if (ImGui::Button("Revive Player"))
			(void)Request_RevivePlayer(m_strActionFeedback);
		ImGui::EndDisabled();
	}

	std::string Status = m_strActionFeedback.empty() ?
		m_strStatus : m_strActionFeedback;
	if (m_strActionFeedback.empty())
	{
		if (!Can_DisplayValtanView(m_eGraphAdmission) || !m_bGraphReady)
			Status = "Play unavailable: canonical Valtan graph did not load.";
		else if (nullptr == pSelected)
			Status = "Play unavailable: select one pattern from the list.";
		else if (!CNetworkManager::Get().Is_Connected())
			Status = "Play unavailable: connect the Debug Server, then enter Valtan Arena.";
		else if (!Boss.isValid)
			Status = "Play unavailable: no replicated Valtan exists in this room.";
		else if (!Player.isValid)
			Status = "Play unavailable: the Server player snapshot is not ready.";
		else if (0u == Player.iCurrentHp)
			Status = "Play paused: revive the player, then Repeat continues from the same selection.";
		else if (!Player.isCombatReady)
			Status = "Play unavailable: wait for the Server player to become combat-ready.";
		else if (bFlowOwnsPlayback)
			Status = "Play unavailable while ordered Pattern Flow is active.";
		else if (bNextOwnsPlayback)
			Status = "Play and Repeat are locked while Next owns a reservation or unresolved command.";
		else if (Audition.Is_InFlight() ||
			(CONSUMER_ID == Audition.strConsumerId &&
			 VALTAN_PATTERN_AUDITION_STATE::IDLE != Audition.eState))
			Status = Audition.strStatus;
	}
	ImGui::TextWrapped("%s", Status.c_str());
}

void Client::CBossTool::Normalize_CurrentFlowSelection()
{
	const VALTAN_PATTERN_FLOW_DEFINITION* const pSavedFlow =
		m_FlowDocument.Get_SavedDefaultFlow();
	if (nullptr == pSavedFlow || pSavedFlow->Slots.empty())
	{
		m_strSelectedCurrentFlowSlotId.clear();
		return;
	}

	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	if (m_bFollowLive &&
		!Playback.strCurrentSlotId.empty() &&
		Playback.strFlowRevision == m_FlowDocument.Get_SourceRevision() &&
		pSavedFlow->Slots.end() != std::find_if(
			pSavedFlow->Slots.begin(), pSavedFlow->Slots.end(),
			[&Playback](const VALTAN_PATTERN_FLOW_SLOT& Slot)
			{
				return Slot.strSlotId == Playback.strCurrentSlotId;
			}))
	{
		m_strSelectedCurrentFlowSlotId = Playback.strCurrentSlotId;
		return;
	}

	const auto Selected = std::find_if(
		pSavedFlow->Slots.begin(), pSavedFlow->Slots.end(),
		[this](const VALTAN_PATTERN_FLOW_SLOT& Slot)
		{
			return Slot.strSlotId == m_strSelectedCurrentFlowSlotId;
		});
	if (pSavedFlow->Slots.end() == Selected)
		m_strSelectedCurrentFlowSlotId = pSavedFlow->Slots.front().strSlotId;
}

void Client::CBossTool::Select_Pattern(const VALTAN_PATTERN_VIEW& Pattern)
{
	const bool_t bSelectionChanged =
		m_strSelectedPatternId != Pattern.strPatternId;
	m_bReviveFeedbackPending = false;
	if (bSelectionChanged && (m_bRepeat || !m_strRepeatPatternId.empty()))
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
		m_strActionFeedback =
			"Repeat stopped after selecting a different Pattern. The current Server pattern will finish normally.";
	}
	else
	{
		m_strActionFeedback.clear();
	}
	m_strSelectedPatternId = Pattern.strPatternId;
#ifdef _DEBUG
	if (CMainApp* const pApp = CMainApp::Get_Active())
		(void)pApp->Debug_SelectCompletePlayPattern(m_strSelectedPatternId);
#endif
	m_strSelectedStageId = Pattern.Stages.empty() ?
		std::string{} : Pattern.Stages.front().strStageId;
	m_bFollowLive = false;
}

void Client::CBossTool::Render_ProductFallbackPatternList()
{
	ImGui::TextUnformatted("Generated Product Patterns");
	ImGui::TextDisabled("Selection is display-only; no Tool command is armed.");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##bossProductFallbackPatternSearch", "Search Product pattern...",
		m_PatternSearch.data(), m_PatternSearch.size());
	const std::string Query = m_PatternSearch.data();
	if (!ImGui::BeginChild(
			"##bossProductFallbackPatternList", ImVec2(0.f, 0.f), true))
	{
		ImGui::EndChild();
		return;
	}

	size_t iVisible = 0u;
	for (const ENCOUNTER_PATTERN_REFERENCE& Pattern :
		m_ProductFallbackEncounterReference.Get_Patterns())
	{
		bool_t bMatches = Contains_CaseInsensitive(Pattern.patternId, Query) ||
			Contains_CaseInsensitive(Pattern.displayName, Query) ||
			Contains_CaseInsensitive(Pattern.actionId, Query);
		if (!bMatches)
		{
			bMatches = std::any_of(
				Pattern.stages.begin(), Pattern.stages.end(),
				[&Query](const ENCOUNTER_STAGE_REFERENCE& Stage)
				{
					return Contains_CaseInsensitive(Stage.stageId, Query) ||
						Contains_CaseInsensitive(Stage.actionId, Query);
				});
		}
		if (!bMatches)
			continue;
		++iVisible;
		ImGui::PushID(Pattern.patternId.c_str());
		if (ImGui::Selectable(
				Pattern.displayName.empty() ? Pattern.patternId.c_str() :
					Pattern.displayName.c_str(),
				m_strSelectedPatternId == Pattern.patternId))
		{
			m_strSelectedPatternId = Pattern.patternId;
			m_strSelectedStageId = Pattern.stages.empty() ?
				std::string{} : Pattern.stages.front().stageId;
			m_bFollowLive = false;
			m_bRepeat = false;
			m_strRepeatPatternId.clear();
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Product Pattern ID: %s\nEntry action: %s",
				Pattern.patternId.c_str(), Pattern.actionId.c_str());
		}
		ImGui::PopID();
	}
	if (0u == iVisible)
		ImGui::TextDisabled("No Product pattern matches this search.");
	ImGui::EndChild();
}

void Client::CBossTool::Render_ProductFallbackSelectedPattern()
{
	const ENCOUNTER_PATTERN_REFERENCE* const pPattern =
		Find_ProductFallbackPattern(m_strSelectedPatternId);
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled("Select a generated Product pattern on the left.");
		return;
	}

	ImGui::TextColored(
		ImVec4(1.f, 0.72f, 0.18f, 1.f),
		"READ-ONLY PRODUCT FALLBACK");
	ImGui::Text("Selected: %s",
		pPattern->displayName.empty() ? pPattern->patternId.c_str() :
			pPattern->displayName.c_str());
	ImGui::TextWrapped("Pattern ID: %s | Entry action: %s",
		pPattern->patternId.c_str(), pPattern->actionId.c_str());
	ImGui::TextDisabled("Target %s | Aim %s | %u ms total",
		pPattern->targetPolicy.c_str(), pPattern->aimPolicy.c_str(),
		pPattern->iTotalDurationMs);
	ImGui::TextWrapped(
		"This generated Product projection can identify patterns and live actions only. Strict split authoring did not pass, so Save, Restart, Play, Repeat, Next, and authoring branch edits remain unavailable.");

	if (pPattern->stages.empty())
		return;
	const ENCOUNTER_STAGE_REFERENCE* pSelectedStage =
		Find_ProductFallbackStage(*pPattern, m_strSelectedStageId);
	if (nullptr == pSelectedStage)
	{
		m_strSelectedStageId = pPattern->stages.front().stageId;
		pSelectedStage = &pPattern->stages.front();
	}
	const std::string Preview = pSelectedStage->stageId + " / " +
		pSelectedStage->actionId;
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo("##bossProductFallbackStage", Preview.c_str()))
	{
		for (const ENCOUNTER_STAGE_REFERENCE& Stage : pPattern->stages)
		{
			const bool_t bSelected = Stage.stageId == m_strSelectedStageId;
			const std::string Label = Stage.stageId + " / " + Stage.actionId;
			if (ImGui::Selectable(Label.c_str(), bSelected))
				m_strSelectedStageId = Stage.stageId;
			if (bSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	pSelectedStage = Find_ProductFallbackStage(
		*pPattern, m_strSelectedStageId);
	if (nullptr != pSelectedStage)
	{
		ImGui::Text("%s | %u ms", pSelectedStage->stageKind.c_str(),
			pSelectedStage->iDurationMs);
		ImGui::TextWrapped("Action: %s", pSelectedStage->actionId.c_str());
		if (pSelectedStage->bHasCounterHitBranch)
			ImGui::TextColored(
				ImVec4(0.36f, 0.86f, 0.96f, 1.f), "COUNTER");
	}
}

void Client::CBossTool::Render_ProductFallbackLogicPattern()
{
	ImGui::TextColored(
		ImVec4(1.f, 0.72f, 0.18f, 1.f),
		"READ-ONLY PRODUCT FALLBACK");
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	if (ImGui::Button("Retry Strict Graph Load##logicProductFallback"))
		(void)Reload_Graph();
	ImGui::Separator();

	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	const bool_t bHasLivePattern = Boss.isValid &&
		!Boss.strPatternId.empty() && nullptr !=
		Find_ProductFallbackPattern(Boss.strPatternId);
	const std::string& PatternId = bHasLivePattern ?
		Boss.strPatternId : m_strSelectedPatternId;
	const ENCOUNTER_PATTERN_REFERENCE* const pPattern =
		Find_ProductFallbackPattern(PatternId);
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled(
			"No live or selected generated Product pattern can be resolved.");
		return;
	}

	const bool_t bHasCounterHitBranch = std::any_of(
		pPattern->stages.begin(), pPattern->stages.end(),
		[](const ENCOUNTER_STAGE_REFERENCE& Stage)
		{ return Stage.bHasCounterHitBranch; });
	ImGui::Text("%s | %s",
		pPattern->displayName.empty() ? pPattern->patternId.c_str() :
			pPattern->displayName.c_str(),
		bHasLivePattern ? "SERVER LIVE" : "SELECTED");
	if (bHasLivePattern)
	{
		ImGui::Text("Sequence %u", Boss.iPatternSequence);
		Render_LogicCounterBadge(bHasCounterHitBranch);
		ImGui::SameLine();
		ImGui::Text("| Stage %u | Tick %u",
			Boss.iPatternStageIndex, Boss.iServerTick);
	}
	else if (bHasCounterHitBranch)
	{
		ImGui::TextColored(
			ImVec4(0.36f, 0.86f, 0.96f, 1.f), "COUNTER");
	}
	ImGui::TextDisabled(
		"Generated Product stage topology only. Conditional branch authoring requires a successful strict split join.");

	if (ImGui::BeginTable(
			"##logicProductFallbackStages", 4,
			ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
			ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Stage");
		ImGui::TableSetupColumn("Kind");
		ImGui::TableSetupColumn("Action");
		ImGui::TableSetupColumn("Duration");
		ImGui::TableHeadersRow();
		for (const ENCOUNTER_STAGE_REFERENCE& Stage : pPattern->stages)
		{
			const bool_t bLive = bHasLivePattern &&
				Stage.actionId == Boss.strActionId;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s%s", bLive ? "> " : "", Stage.stageId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(Stage.stageKind.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextWrapped("%s", Stage.actionId.c_str());
			if (Stage.bHasCounterHitBranch)
			{
				ImGui::SameLine();
				ImGui::TextColored(
					ImVec4(0.36f, 0.86f, 0.96f, 1.f), "COUNTER");
			}
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u ms", Stage.iDurationMs);
		}
		ImGui::EndTable();
	}

	if (bHasLivePattern &&
		nullptr == Find_ProductFallbackStage(*pPattern, Boss.strActionId))
	{
		ImGui::TextWrapped(
			"Live action is not present in the generated Product fallback: %s",
			Boss.strActionId.c_str());
	}
}

void Client::CBossTool::Render_CurrentPatternList()
{
	Normalize_CurrentFlowSelection();
	ImGui::TextUnformatted("Saved Gameplay Sequence");
	const VALTAN_PATTERN_FLOW_DEFINITION* const pSavedFlow =
		m_FlowDocument.Get_SavedDefaultFlow();
	if (nullptr == pSavedFlow || pSavedFlow->Slots.empty())
	{
		ImGui::TextWrapped(
			"No canonical scriptedSequence is loaded. Add Patterns in Pattern Flow, then Save Flow.");
		return;
	}

	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const bool_t bPlaybackMatchesSavedRevision =
		!Playback.strFlowRevision.empty() &&
		Playback.strFlowRevision == m_FlowDocument.Get_SourceRevision();
	if (m_FlowDocument.Is_Dirty())
	{
		ImGui::TextDisabled(
			"Unsaved Pattern Flow edits are hidden here. Save Flow updates Valtan.gameplay.json and Current Patterns.");
	}
	if (Playback.Is_InFlight() && !bPlaybackMatchesSavedRevision)
	{
		ImGui::TextDisabled(
			"The previous gameplay revision is running. Restart Saved Flow (Fresh Arena) uses the latest complete saved order below and restores the arena before Pattern 01.");
	}

	if (!ImGui::BeginChild(
			"##bossCurrentPatternList", ImVec2(0.f, 0.f), true))
	{
		ImGui::EndChild();
		return;
	}
	for (std::size_t Index = 0u; Index < pSavedFlow->Slots.size(); ++Index)
	{
		const VALTAN_PATTERN_FLOW_SLOT& Slot = pSavedFlow->Slots[Index];
		const VALTAN_PATTERN_VIEW* const pPattern =
			Find_AuditionPattern(Slot.strPatternId);
		const bool_t bPlaying = bPlaybackMatchesSavedRevision &&
			Playback.strCurrentSlotId == Slot.strSlotId &&
			(Playback.Is_InFlight() || Playback.Is_TerminalHold());
		std::string Label =
			(Index < 9u ? "0" : "") + std::to_string(Index + 1u) + "  " +
			(nullptr != pPattern && !pPattern->strDisplayName.empty() ?
				pPattern->strDisplayName : Slot.strPatternId);
		if (bPlaying)
			Label += "  [PLAYING]";

		ImGui::PushID(Slot.strSlotId.c_str());
		if (ImGui::Selectable(
				Label.c_str(),
				m_strSelectedCurrentFlowSlotId == Slot.strSlotId) &&
			nullptr != pPattern)
		{
			m_strSelectedCurrentFlowSlotId = Slot.strSlotId;
			Select_Pattern(*pPattern);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Saved slot: %s\nPattern: %s",
				Slot.strSlotId.c_str(), Slot.strPatternId.c_str());
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void Client::CBossTool::Render_PatternList()
{
	/* Selection made in Effect/Workbench/F1 is reflected here before the list
	   renders.  Repeat is a Boss-only lifecycle and must not silently continue
	   against a different shared selection. */
#ifdef _DEBUG
	if (CMainApp* const pApp = CMainApp::Get_Active())
	{
		const std::string& strSharedPatternId =
			pApp->Debug_GetSelectedCompletePlayPatternId();
		const VALTAN_PATTERN_VIEW* pShared =
			Find_AuditionPattern(strSharedPatternId);
		if (nullptr != pShared &&
			m_strSelectedPatternId != strSharedPatternId)
		{
			m_strSelectedPatternId = strSharedPatternId;
			m_strSelectedStageId = pShared->Stages.empty() ?
				std::string{} : pShared->Stages.front().strStageId;
			m_bFollowLive = false;
			if (m_bRepeat || !m_strRepeatPatternId.empty())
			{
				m_bRepeat = false;
				m_strRepeatPatternId.clear();
				m_strActionFeedback =
					"Repeat stopped after another Tool selected a different Pattern.";
			}
		}
	}
#endif
	ImGui::TextUnformatted("Patterns");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##bossPatternSearch",
		"Search pattern...",
		m_PatternSearch.data(),
		m_PatternSearch.size());

	const std::string Query = m_PatternSearch.data();
	size_t iVisiblePatternCount = 0u;
	if (!ImGui::BeginChild(
			"##bossPatternList", ImVec2(0.f, 0.f), true))
	{
		ImGui::EndChild();
		return;
	}
	const auto Matches = [&Query](const VALTAN_PATTERN_VIEW& Pattern)
	{
		bool_t bMatches = Contains_CaseInsensitive(
			Pattern.strPatternId, Query) ||
			Contains_CaseInsensitive(Pattern.strDisplayName, Query) ||
			Contains_CaseInsensitive(Pattern.strActionId, Query);
		if (!bMatches)
		{
			bMatches = std::any_of(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[&Query](const VALTAN_STAGE_VIEW& Stage)
				{
					return Contains_CaseInsensitive(
						Stage.strActionId, Query);
				});
		}
		return bMatches;
	};
	const auto IsVisible = [this, &Matches](const VALTAN_PATTERN_VIEW& Pattern)
	{
		return Matches(Pattern) ||
			(m_bFollowLive && Pattern.strPatternId == m_strLivePatternId);
	};
	const auto HasVisible = [this, &IsVisible](
		const std::vector<std::string>& PatternIds)
	{
		return std::any_of(
			PatternIds.begin(), PatternIds.end(),
			[this, &IsVisible](const std::string& strPatternId)
			{
				const VALTAN_PATTERN_VIEW* pPattern =
					Find_AuditionPattern(strPatternId);
				return nullptr != pPattern && IsVisible(*pPattern);
			});
	};
	const auto RenderPatternIds =
		[this, &IsVisible, &iVisiblePatternCount](
			const std::vector<std::string>& PatternIds)
	{
		for (const std::string& strPatternId : PatternIds)
		{
			const VALTAN_PATTERN_VIEW* pPattern =
				Find_AuditionPattern(strPatternId);
			if (nullptr == pPattern || !IsVisible(*pPattern))
				continue;
			++iVisiblePatternCount;
			const bool_t bLive =
				pPattern->strPatternId == m_strLivePatternId;
			std::string Label = pPattern->strDisplayName.empty() ?
				pPattern->strPatternId : pPattern->strDisplayName;
			if (bLive)
				Label += "  [LIVE]";

			ImGui::PushID(pPattern->strPatternId.c_str());
			if (bLive)
				ImGui::PushStyleColor(
					ImGuiCol_Text, ImVec4(0.30f, 0.92f, 1.f, 1.f));
			if (ImGui::Selectable(
					Label.c_str(),
					m_strSelectedPatternId == pPattern->strPatternId))
			{
				Select_Pattern(*pPattern);
			}
			if (bLive)
				ImGui::PopStyleColor();
			if (bLive && m_bFollowLive &&
				m_strLastAutoRevealedLivePatternId != pPattern->strPatternId)
			{
				ImGui::SetScrollHereY(0.5f);
				m_strLastAutoRevealedLivePatternId = pPattern->strPatternId;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s",
					CValtanPatternTree::Build_PatternIdentitySummary(*pPattern).c_str());
			}
			ImGui::PopID();
		}
	};
	if (HasVisible(m_AuditionInventory.CorePatternIds))
	{
		ImGui::SeparatorText((std::string("CORE SERVER PATTERNS (") +
			std::to_string(m_AuditionInventory.CorePatternIds.size()) + ")").c_str());
		RenderPatternIds(m_AuditionInventory.CorePatternIds);
	}
	if (HasVisible(m_AuditionInventory.AnimatorPatternIds))
	{
		ImGui::SeparatorText((std::string("ANIMATOR PATTERNS (") +
			std::to_string(m_AuditionInventory.AnimatorPatternIds.size()) + ")").c_str());
		RenderPatternIds(m_AuditionInventory.AnimatorPatternIds);
	}
	if (HasVisible(m_AuditionInventory.DerivedPatternIds))
	{
		ImGui::SeparatorText((std::string("DERIVED SERVER PATTERNS (") +
			std::to_string(m_AuditionInventory.DerivedPatternIds.size()) + ")").c_str());
		RenderPatternIds(m_AuditionInventory.DerivedPatternIds);
	}
	if (0u == iVisiblePatternCount)
		ImGui::TextDisabled("No pattern matches this search.");
	ImGui::EndChild();
}

void Client::CBossTool::Render_SelectedPattern()
{
	const VALTAN_PATTERN_VIEW* pPattern =
		Find_AuditionPattern(m_strSelectedPatternId);
	if (nullptr == pPattern)
	{
		ImGui::TextUnformatted("Select a pattern on the left.");
		return;
	}

	ImGui::Text(
		"Selected: %s",
		pPattern->strDisplayName.empty() ?
			pPattern->strPatternId.c_str() :
			pPattern->strDisplayName.c_str());
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", pPattern->strPatternId.c_str());
	ImGui::TextWrapped("%s",
		CValtanPatternTree::Build_PatternIdentitySummary(*pPattern).c_str());
	if (0 != pPattern->iTriggerHealthBar)
	{
		ImGui::Text(
			"Gate: %d health bars | Phase %u-%u",
			pPattern->iTriggerHealthBar,
			pPattern->iMinimumPhase,
			pPattern->iMaximumPhase);
	}
	else
	{
		ImGui::TextDisabled(
			"Runtime phase %u-%u | Selection mode: %s",
			pPattern->iMinimumPhase,
			pPattern->iMaximumPhase, pPattern->strSelectionMode.c_str());
	}
	const HUD_BOSS_STATE& LiveBoss = CCombatHUDViewModel::Get().Get_Boss();
	if (LiveBoss.isValid &&
		LiveBoss.strPatternId == pPattern->strPatternId &&
		LiveBoss.PinnedDefinitionRevision.Is_Valid())
	{
		const bool_t bRevisionAvailable =
			CNetworkManager::Get().Is_PresentationRevisionAvailable(
				LiveBoss.PinnedDefinitionRevision);
		if (!bRevisionAvailable || !m_bPresentationBaselineIntact)
		{
			ImGui::TextWrapped(
				"Connections are unverified: %s",
				bRevisionAvailable ?
					m_strPresentationFreshnessStatus.c_str() :
					"the Server-pinned presentation revision is unavailable locally.");
		}
	}
	if (pPattern->Stages.empty())
	{
		ImGui::TextUnformatted("This pattern has no admitted Server stage.");
		return;
	}

	const VALTAN_STAGE_VIEW* pStage = Find_SelectedStage(*pPattern);
	if (nullptr == pStage)
	{
		m_strSelectedStageId = pPattern->Stages.front().strStageId;
		pStage = &pPattern->Stages.front();
	}
	std::string Preview = pStage->strStageId + " / " + pStage->strActionId;
	if (pPattern->strPatternId == m_strLivePatternId &&
		pStage->strStageId == m_strLiveStageId)
	{
		Preview += "  [LIVE]";
	}
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo("##bossStage", Preview.c_str()))
	{
		for (const VALTAN_STAGE_VIEW& Stage : pPattern->Stages)
		{
			ImGui::PushID(Stage.strStageId.c_str());
			const bool_t bSelected =
				Stage.strStageId == m_strSelectedStageId;
			const bool_t bLiveStage =
				pPattern->strPatternId == m_strLivePatternId &&
				Stage.strStageId == m_strLiveStageId;
			std::string Label = Stage.strStageId + " / " + Stage.strActionId;
			if (bLiveStage)
				Label += "  [LIVE]";
			if (bLiveStage)
				ImGui::PushStyleColor(
					ImGuiCol_Text, ImVec4(0.30f, 0.92f, 1.f, 1.f));
			if (ImGui::Selectable(Label.c_str(), bSelected))
			{
				m_strSelectedStageId = Stage.strStageId;
				m_bFollowLive = false;
			}
			if (bLiveStage)
				ImGui::PopStyleColor();
			if (bSelected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	pStage = Find_SelectedStage(*pPattern);
	if (nullptr == pStage)
		return;
	ImGui::TextDisabled(
		"Gameplay rows: LOCAL AUTHORING - Server parity unverified.");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(
			"Gate, hit, motion, world, and next-edge values come from the "
			"current local gameplay authoring. Equality with the "
			"Server-pinned gameplay generation is not proven.");
	}
	if (pPattern->strPatternId == m_strLivePatternId)
	{
		ImGui::TextColored(
			ImVec4(0.30f, 0.92f, 1.f, 1.f),
			"LIVE: %s / %s",
			m_strLivePatternId.c_str(),
			m_strLiveStageId.empty() ?
				"stage unresolved" : m_strLiveStageId.c_str());
	}
	Render_SelectedPatternRingAuthoring(*pPattern);
	Render_ConnectionSummary(*pPattern, *pStage);
}

void Client::CBossTool::Render_SelectedPatternRingAuthoring(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	std::size_t iRingCount = 0u;
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		if ("RING" == Stage.strHitShape)
			++iRingCount;
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
			Stage.CombatObjectEffects)
		{
			iRingCount += static_cast<std::size_t>(std::count_if(
				Object.Hits.begin(), Object.Hits.end(),
				[](const VALTAN_COMBAT_OBJECT_HIT_VIEW& Hit)
				{ return "RING" == Hit.strHitShape; }));
		}
	}
	if (0u == iRingCount)
		return;

	ImGui::SeparatorText("Canonical Donut / Ring Geometry");
	ImGui::TextWrapped(
		"These numeric slots edit the exact Server-owned RING hit. "
		"Pattern, Stage, combat-object and hit IDs remain read-only.");
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled(
			"The shared canonical Valtan draft owner is unavailable.");
		return;
	}

	const double Step = 0.1;
	const double FastStep = 1.0;
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		if ("RING" == Stage.strHitShape)
		{
			CBalanceTool::PATTERN_STAGE_EDIT Draft;
			std::string Status;
			ImGui::PushID((Stage.strStageId + "/stage-ring").c_str());
			if (!m_pBalanceTool->Get_ValtanStageDraft(
					Pattern.strPatternId, Stage.strStageId, Draft, Status) ||
				"RING" != Draft.hitShape)
			{
				ImGui::TextDisabled(
					"Stage RING unavailable: %s", Status.c_str());
				ImGui::PopID();
				continue;
			}
			ImGui::Text("Stage hit | %s / %s",
				Stage.strStageId.c_str(), Stage.strActionId.c_str());
			ImGui::SetNextItemWidth(180.f);
			if (ImGui::InputDouble(
					"Inner radius m", &Draft.hitInnerRadius,
					Step, FastStep, "%.3f") &&
				!m_pBalanceTool->Set_ValtanStageDraft(
					Pattern.strPatternId, Stage.strStageId, Draft, Status))
			{
				m_strActionFeedback = Status;
			}
			else if (!Status.empty())
			{
				m_strActionFeedback = Status;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(180.f);
			Status.clear();
			if (ImGui::InputDouble(
					"Outer radius m", &Draft.hitOuterRadius,
					Step, FastStep, "%.3f") &&
				!m_pBalanceTool->Set_ValtanStageDraft(
					Pattern.strPatternId, Stage.strStageId, Draft, Status))
			{
				m_strActionFeedback = Status;
			}
			else if (!Status.empty())
			{
				m_strActionFeedback = Status;
			}
			ImGui::PopID();
		}

		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
			Stage.CombatObjectEffects)
		{
			for (const VALTAN_COMBAT_OBJECT_HIT_VIEW& Hit : Object.Hits)
			{
				if ("RING" != Hit.strHitShape)
					continue;
				CBalanceTool::VALTAN_COMBAT_OBJECT_RING_HIT_EDIT Draft;
				std::string Status;
				ImGui::PushID((Stage.strStageId + "/" +
					Object.strCombatObjectArchetypeId + "/" + Hit.strHitId).c_str());
				if (!m_pBalanceTool->Get_ValtanCombatObjectRingHitDraft(
						Pattern.strPatternId, Stage.strStageId,
						Object.strCombatObjectArchetypeId, Hit.strHitId,
						Draft, Status))
				{
					ImGui::TextDisabled(
						"Combat-object RING unavailable: %s", Status.c_str());
					ImGui::PopID();
					continue;
				}
				ImGui::TextWrapped(
					"Combat object | %s / %s / %s",
					Stage.strStageId.c_str(),
					Object.strCombatObjectArchetypeId.c_str(),
					Hit.strHitId.c_str());
				ImGui::SetNextItemWidth(180.f);
				if (ImGui::InputDouble(
						"Inner radius m", &Draft.innerRadiusM,
						Step, FastStep, "%.3f") &&
					!m_pBalanceTool->Set_ValtanCombatObjectRingHitDraft(
						Pattern.strPatternId, Stage.strStageId, Draft, Status))
				{
					m_strActionFeedback = Status;
				}
				else if (!Status.empty())
				{
					m_strActionFeedback = Status;
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth(180.f);
				Status.clear();
				if (ImGui::InputDouble(
						"Outer radius m", &Draft.outerRadiusM,
						Step, FastStep, "%.3f") &&
					!m_pBalanceTool->Set_ValtanCombatObjectRingHitDraft(
						Pattern.strPatternId, Stage.strStageId, Draft, Status))
				{
					m_strActionFeedback = Status;
				}
				else if (!Status.empty())
				{
					m_strActionFeedback = Status;
				}
				ImGui::PopID();
			}
		}
	}

	std::string SourceRevision;
	std::string StateStatus;
	bool_t bDirty = false;
	const bool_t bStateReady = m_pBalanceTool->Get_ValtanAuthoringState(
		SourceRevision, bDirty, StateStatus);
	ImGui::BeginDisabled(!bStateReady || !bDirty);
	if (ImGui::Button("Save Canonical Ring Geometry"))
		(void)Save_SelectedPatternRingAuthoring();
	ImGui::EndDisabled();
	if (bStateReady)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("%s | source %.12s",
			bDirty ? "UNSAVED" : "SAVED", SourceRevision.c_str());
	}
	else
	{
		ImGui::TextDisabled("Save unavailable: %s", StateStatus.c_str());
	}
	if (!m_strActionFeedback.empty())
		ImGui::TextWrapped("%s", m_strActionFeedback.c_str());
}

bool_t Client::CBossTool::Save_SelectedPatternRingAuthoring()
{
	std::string Status;
	if (!Can_MutateCanonicalGraph(Status))
	{
		m_strActionFeedback = std::move(Status);
		return false;
	}
	if (nullptr == m_pBalanceTool)
	{
		m_strActionFeedback =
			"Ring Save failed: the shared canonical Valtan draft owner is unavailable.";
		return false;
	}
	if (!m_pBalanceTool->Save_ValtanCanonicalProduct(Status))
	{
		m_strActionFeedback =
			"Canonical Ring Save failed atomically: " + Status;
		return false;
	}
	const std::string SavedStatus = Status;
	const bool_t bCanonicalReopened =
		0u != SavedStatus.rfind("COMMIT_SUCCEEDED_REOPEN_FAILED:", 0u);
	std::string ActivationStatus;
	const bool_t bActivationPrepared = bCanonicalReopened &&
		m_pBalanceTool->Save_ValtanProduct(ActivationStatus);
	const bool_t bReloaded = Reload_Graph();
	m_strActionFeedback =
		"Canonical RING geometry was saved through the shared CAS transaction. " +
		(bReloaded ? std::string("Boss Verification reloaded the saved revision. ") :
			std::string("The physical Save succeeded, but Boss Verification could not reload it yet. ")) +
		SavedStatus + " " +
		(bActivationPrepared ? ActivationStatus :
			(bCanonicalReopened ?
				"Product candidate publication/activation preparation failed: " +
					ActivationStatus :
				"Product candidate publication was skipped until the committed revision can reopen."));
	return true;
}

void Client::CBossTool::Render_ConnectionSummary(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	ImGui::Spacing();
	if (!ImGui::BeginTable(
			"##bossConnections",
			2,
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_BordersInnerH |
			ImGuiTableFlags_SizingStretchProp))
	{
		return;
	}
	ImGui::TableSetupColumn(
		"Lane", ImGuiTableColumnFlags_WidthFixed, 92.f);
	ImGui::TableSetupColumn("Connection", ImGuiTableColumnFlags_WidthStretch);

	std::vector<std::string> Animations;
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
		Append_Unique(Animations, Clip.strClipName);
	const std::string Animation = Stage.bSuppressAnimation ?
		"No body animation (keep current pose)" :
		(Animations.empty() ? "Missing animation binding" : Join(Animations));
	Render_ConnectionRow("Animation", Animation);

	std::vector<std::string> Effects;
	for (const VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
		Append_Unique(Effects, Effect.strEffectAssetId);
	for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
		Append_Unique(Effects, Cue.strEffectAssetId);
	for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
		Stage.CombatObjectEffects)
	{
		Append_Unique(
			Effects,
			Object.strEffectAssetId + " [combat object]");
	}
	for (const std::string& IndependentId : Stage.IndependentEffectIds)
	{
		const VALTAN_INDEPENDENT_EFFECT_VIEW* pIndependent =
			Find_IndependentEffect(IndependentId);
		if (nullptr != pIndependent)
			Append_Unique(Effects, pIndependent->strEffectAssetId);
	}
	Render_ConnectionRow("Effect", Join(Effects));

	std::vector<std::string> Cameras;
	if (!m_CameraDocument.Is_Ready() && !Stage.CameraInvocations.empty())
	{
		Cameras.push_back(m_strCameraStatus);
	}
	else
	{
		for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
			Stage.CameraInvocations)
		{
			const VALTAN_CINEMATIC_CAMERA_CUE* pCue =
				Find_CameraCue(Invocation.strCameraCueId);
			Append_Unique(
				Cameras,
				nullptr == pCue ?
					"Missing cue: " + Invocation.strCameraCueId :
					(Is_ExactCameraInvocation(
						Pattern, Stage, Invocation, *pCue) ?
						Invocation.strCameraCueId + " [" +
							Camera_FrameLabel(pCue->eTrackingMode) + "]" :
						"Tuple mismatch: " + Invocation.strCameraCueId));
		}
	}
	Render_ConnectionRow("Camera", Join(Cameras));

	std::vector<std::string> HitMotion;
	if (Stage.Has_HitShape())
	{
		std::string Hit = Stage.strHitShape;
		if (!Stage.strServerDamageProfileId.empty())
			Hit += " / " + Stage.strServerDamageProfileId;
		if (0u != Stage.iHitCount)
			Hit += " / " + std::to_string(Stage.iHitCount) + " hit(s)";
		Append_Unique(HitMotion, Hit);
	}
	if (Stage.Motion.has_value())
	{
		Append_Unique(
			HitMotion,
			Stage.Motion->strKind + " / " +
				std::to_string(Stage.Motion->fDistance) + " m");
	}
	if (Pattern.ServerMotion.has_value())
	{
		Append_Unique(
			HitMotion,
			"pattern " + Pattern.ServerMotion->strKind + " -> " +
				Pattern.ServerMotion->strAnchorId);
	}
	Render_ConnectionRow("Hit / Motion", Join(HitMotion));

	std::vector<std::string> World;
	for (const VALTAN_STAGE_ACTION_VIEW& Action : Stage.Actions)
	{
		Append_Unique(
			World,
			Action.strKind + (Action.strTargetId.empty() ?
				std::string{} : " -> " + Action.strTargetId));
	}
	for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Reference :
		Pattern.WorldEventTriggerRefs)
	{
		if (Reference.strStageId == Stage.strStageId)
			Append_Unique(World, Reference.strTriggerKind);
	}
	Render_ConnectionRow("World", Join(World));

	std::vector<std::string> Next;
	for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
	{
		Next.push_back(
			Branch.strOutcome + " -> " +
			(Branch.strNextActionId.has_value() ?
				*Branch.strNextActionId : "terminal"));
	}
	if (Next.empty())
	{
		const auto Current = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&Stage](const VALTAN_STAGE_VIEW& Candidate)
			{
				return Candidate.strStageId == Stage.strStageId;
			});
		if (Current != Pattern.Stages.end() &&
			std::next(Current) != Pattern.Stages.end())
		{
			Next.push_back("fallthrough -> " + std::next(Current)->strActionId);
		}
		else
		{
			Next.push_back("terminal");
		}
	}
	Render_ConnectionRow("Next", Join(Next));
	ImGui::EndTable();
	for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
	{
		ImGui::PushID(Cue.strOccurrenceId.c_str());
		if (ImGui::Button("Edit Linked Effect"))
		{
			m_strEffectToolOpenPatternId = Pattern.strPatternId;
			m_strEffectToolOpenStageId = Stage.strStageId;
			m_strEffectToolOpenCueOccurrenceId = Cue.strOccurrenceId;
			m_strEffectToolOpenEffectAssetId = Cue.strEffectAssetId;
			m_hasEffectToolOpenRequest = true;
			m_strActionFeedback =
				"Opening exact Product Effect: " + Cue.strEffectAssetId;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"%s\n%s\n%s",
				Cue.strEffectAssetId.c_str(),
				Cue.strOccurrenceId.c_str(),
				Cue.bUsesStageClock ?
					"Stage-clock cue: Effect Tool opens a static Valtan target with no animation timeline." :
					"Clip-bound cue: Effect Tool opens the complete synchronized Product timeline.");
		}
		ImGui::PopID();
	}
	for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
		Stage.CameraInvocations)
	{
		if (nullptr == Find_CameraCue(Invocation.strCameraCueId))
			continue;
		const std::string label =
			"Open Camera Tool##" + Invocation.strCameraCueId;
		if (ImGui::Button(label.c_str()))
		{
			m_CameraToolOpenRequest.strCueId = Invocation.strCameraCueId;
			m_hasCameraToolOpenRequest = true;
			m_strActionFeedback =
				"Opening Camera Tool: " + Invocation.strCameraCueId;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", Invocation.strCameraCueId.c_str());
	}
}

bool_t Client::CBossTool::Consume_CameraToolOpenRequest(
	CAMERA_TOOL_OPEN_REQUEST& outRequest)
{
	if (!m_hasCameraToolOpenRequest)
		return false;
	outRequest = std::move(m_CameraToolOpenRequest);
	m_CameraToolOpenRequest = CAMERA_TOOL_OPEN_REQUEST{};
	m_hasCameraToolOpenRequest = false;
	return true;
}

bool_t Client::CBossTool::Consume_EffectToolOpenRequest(
	EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& outRequest)
{
	if (!m_hasEffectToolOpenRequest)
		return false;
	outRequest.strPatternId = std::move(m_strEffectToolOpenPatternId);
	outRequest.strStageId = std::move(m_strEffectToolOpenStageId);
	outRequest.strCueOccurrenceId =
		std::move(m_strEffectToolOpenCueOccurrenceId);
	outRequest.strEffectAssetId =
		std::move(m_strEffectToolOpenEffectAssetId);
	m_strEffectToolOpenPatternId.clear();
	m_strEffectToolOpenStageId.clear();
	m_strEffectToolOpenCueOccurrenceId.clear();
	m_strEffectToolOpenEffectAssetId.clear();
	m_hasEffectToolOpenRequest = false;
	return true;
}

const Client::VALTAN_PATTERN_VIEW* Client::CBossTool::Find_Pattern(
	const std::string& strPatternId) const
{
	if (strPatternId.empty())
		return nullptr;
	const auto FindIn = [&strPatternId](
		const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
	{
		return std::find_if(
			Patterns.begin(), Patterns.end(),
			[&strPatternId](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == strPatternId;
			});
	};
	const auto Gimmick = FindIn(m_Graph.Gimmicks);
	if (Gimmick != m_Graph.Gimmicks.end())
		return &*Gimmick;
	const auto Rotation = FindIn(m_Graph.Rotation);
	return Rotation == m_Graph.Rotation.end() ? nullptr : &*Rotation;
}

const Client::VALTAN_PATTERN_VIEW* Client::CBossTool::Find_AuditionPattern(
	const std::string& strPatternId) const
{
	if (!m_AuditionInventory.Contains(strPatternId))
		return nullptr;
	return Find_Pattern(strPatternId);
}

const Client::ENCOUNTER_PATTERN_REFERENCE*
Client::CBossTool::Find_ProductFallbackPattern(
	const std::string& strPatternId) const
{
	if (!Can_DisplayValtanView(m_eGraphAdmission) ||
		!m_bProductFallbackReady || strPatternId.empty())
		return nullptr;
	return m_ProductFallbackEncounterReference.Find_Pattern(strPatternId);
}

const Client::ENCOUNTER_STAGE_REFERENCE*
Client::CBossTool::Find_ProductFallbackStage(
	const ENCOUNTER_PATTERN_REFERENCE& Pattern,
	const std::string& strActionId) const
{
	if (strActionId.empty())
		return nullptr;
	const auto Stage = std::find_if(
		Pattern.stages.begin(), Pattern.stages.end(),
		[&strActionId](const ENCOUNTER_STAGE_REFERENCE& Candidate)
		{
			return Candidate.actionId == strActionId ||
				Candidate.stageId == strActionId;
		});
	return Pattern.stages.end() == Stage ? nullptr : &*Stage;
}

const Client::VALTAN_PATTERN_FLOW_SLOT*
Client::CBossTool::Find_SelectedFlowSlot() const
{
	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow || m_strSelectedFlowSlotId.empty())
		return nullptr;
	const auto Slot = std::find_if(
		pFlow->Slots.begin(), pFlow->Slots.end(),
		[this](const VALTAN_PATTERN_FLOW_SLOT& Candidate)
		{
			return Candidate.strSlotId == m_strSelectedFlowSlotId;
		});
	return Slot == pFlow->Slots.end() ? nullptr : &*Slot;
}

const Client::VALTAN_PATTERN_FLOW_NODE*
Client::CBossTool::Find_SelectedFlowNode() const
{
	const VALTAN_PATTERN_FLOW_DEFINITION* const pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow || m_strSelectedFlowSlotId.empty())
		return nullptr;
	const auto Node = std::find_if(
		pFlow->Nodes.begin(), pFlow->Nodes.end(),
		[this](const VALTAN_PATTERN_FLOW_NODE& Candidate)
		{
			return Candidate.strNodeId == m_strSelectedFlowSlotId;
		});
	return Node == pFlow->Nodes.end() ? nullptr : &*Node;
}

const Client::VALTAN_PATTERN_FLOW_EDGE*
Client::CBossTool::Find_SelectedFlowEdge() const
{
	const VALTAN_PATTERN_FLOW_DEFINITION* const pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow || m_strSelectedFlowEdgeId.empty())
		return nullptr;
	const auto Edge = std::find_if(
		pFlow->Edges.begin(), pFlow->Edges.end(),
		[this](const VALTAN_PATTERN_FLOW_EDGE& Candidate)
		{
			return Candidate.strEdgeId == m_strSelectedFlowEdgeId;
		});
	return Edge == pFlow->Edges.end() ? nullptr : &*Edge;
}

std::vector<std::string> Client::CBossTool::Build_AdmittedPatternIds() const
{
	std::vector<std::string> PatternIds;
	PatternIds.reserve(m_AuditionInventory.Get_PatternCount());
	PatternIds.insert(
		PatternIds.end(),
		m_AuditionInventory.CorePatternIds.begin(),
		m_AuditionInventory.CorePatternIds.end());
	PatternIds.insert(
		PatternIds.end(),
		m_AuditionInventory.AnimatorPatternIds.begin(),
		m_AuditionInventory.AnimatorPatternIds.end());
	PatternIds.insert(
		PatternIds.end(),
		m_AuditionInventory.DerivedPatternIds.begin(),
		m_AuditionInventory.DerivedPatternIds.end());
	return PatternIds;
}

const Client::VALTAN_STAGE_VIEW* Client::CBossTool::Find_LiveStage(
	const VALTAN_PATTERN_VIEW& Pattern) const
{
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!Boss.isValid || Boss.strPatternId != Pattern.strPatternId ||
		Boss.strActionId.empty())
	{
		return nullptr;
	}
	const auto Stage = std::find_if(
		Pattern.Stages.begin(), Pattern.Stages.end(),
		[&Boss](const VALTAN_STAGE_VIEW& Candidate)
		{
			return Candidate.strActionId == Boss.strActionId;
		});
	return Stage == Pattern.Stages.end() ? nullptr : &*Stage;
}

const Client::VALTAN_STAGE_VIEW* Client::CBossTool::Find_SelectedStage(
	const VALTAN_PATTERN_VIEW& Pattern) const
{
	const auto Stage = std::find_if(
		Pattern.Stages.begin(), Pattern.Stages.end(),
		[this](const VALTAN_STAGE_VIEW& Candidate)
		{
			return Candidate.strStageId == m_strSelectedStageId;
		});
	return Stage == Pattern.Stages.end() ? nullptr : &*Stage;
}

const Client::VALTAN_INDEPENDENT_EFFECT_VIEW*
Client::CBossTool::Find_IndependentEffect(
	const std::string& strIndependentEffectId) const
{
	const auto Effect = std::find_if(
		m_Graph.IndependentEffects.begin(), m_Graph.IndependentEffects.end(),
		[&strIndependentEffectId](
			const VALTAN_INDEPENDENT_EFFECT_VIEW& Candidate)
		{
			return Candidate.strIndependentEffectId == strIndependentEffectId;
		});
	return Effect == m_Graph.IndependentEffects.end() ? nullptr : &*Effect;
}

const Client::VALTAN_CINEMATIC_CAMERA_CUE*
Client::CBossTool::Find_CameraCue(const std::string& strCueId) const
{
	const auto Cue = std::find_if(
		m_CameraDocument.Get_Cues().begin(),
		m_CameraDocument.Get_Cues().end(),
		[&strCueId](const VALTAN_CINEMATIC_CAMERA_CUE& Candidate)
		{
			return Candidate.strCueId == strCueId;
		});
	return Cue == m_CameraDocument.Get_Cues().end() ? nullptr : &*Cue;
}
