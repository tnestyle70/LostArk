#include "imgui.h"

#include "BossTool.h"

#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "NetworkManager.h"
#include "PlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPatternFlowService.h"

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
	std::shared_ptr<IPlayerCommandSink> CommandSink)
	: m_pCommandSink(std::move(CommandSink))
{
}

void Client::CBossTool::Open()
{
	m_bOpen = true;
	m_bFocusPending = true;
	if (!m_bGraphLoadAttempted)
		(void)Reload_Graph();
}

void Client::CBossTool::Update(const bool_t bToolVisible)
{
	if (!bToolVisible || !m_bOpen)
	{
		m_bRepeat = false;
		m_strRepeatPatternId.clear();
		return;
	}

	Synchronize_LiveSelection();
	Refresh_PresentationFreshness();
	const VALTAN_PATTERN_FLOW_SNAPSHOT& FlowSnapshot =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	if (m_bReviveFeedbackPending && Player.isValid && 0u != Player.iCurrentHp)
	{
		m_bReviveFeedbackPending = false;
		m_strActionFeedback.clear();
	}
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Audition =
		CValtanPatternAuditionService::Get().Get_Snapshot();
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
		FlowSnapshot.Is_InFlight() ||
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

bool_t Client::CBossTool::Reload_Graph()
{
	m_bReviveFeedbackPending = false;
	m_strActionFeedback.clear();
	m_bGraphLoadAttempted = true;
	VALTAN_PATTERN_TREE_VIEW StagedGraph;
	std::string Status;
	if (!CValtanPatternTree::Load(StagedGraph, Status))
	{
		m_strStatus = "Graph reload failed: " + Status;
		return false;
	}
	VALTAN_TOOL_AUDITION_INVENTORY StagedAuditionInventory;
	std::string InventoryError;
	if (!CValtanPatternTree::Build_ToolAuditionInventory(
			StagedGraph, StagedAuditionInventory, InventoryError))
	{
		m_strStatus = "Graph reload failed: " + InventoryError;
		return false;
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

	m_Graph = std::move(StagedGraph);
	m_AuditionInventory = std::move(StagedAuditionInventory);
	m_EncounterReference = bEncounterReady ?
		std::move(StagedEncounter) : CEncounterPatternReference{};
	m_CameraDocument = bCameraReady ?
		std::move(StagedCamera) : CValtanCinematicCameraDocument{};
	m_strCameraStatus = bCameraReady ?
		"Camera cues loaded." :
		"Camera lane unavailable: " +
			(bEncounterReady ? CameraStatus : EncounterStatus);
	m_EffectDocuments.clear();
	m_ResourceSearchDocumentGenerations.clear();
	m_ResourceOwnerResults.clear();
	m_strDiagnosticStatus.clear();
	m_iResourceSearchLoadFailureCount = 0u;
	m_iResourceSearchUnverifiedCount = 0u;
	m_dNextResourceSearchFreshnessCheckSeconds = 0.0;
	m_bResourceSearchStale = false;
	m_bGraphReady = true;
	Refresh_PresentationFreshness(true);
	if (!m_FlowDocument.Is_Ready())
	{
		std::string FlowStatus;
		if (m_FlowDocument.Load(Build_AdmittedPatternIds(), FlowStatus))
		{
			m_strFlowStatus = FlowStatus;
			const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
				m_FlowDocument.Get_DefaultFlow();
			if (nullptr != pFlow && !pFlow->Slots.empty())
				m_strSelectedFlowSlotId = pFlow->Slots.front().strSlotId;
		}
		else
		{
			m_strFlowStatus = "Flow load failed: " + FlowStatus;
		}
	}
	else
	{
		std::string FlowValidationStatus;
		if (!CValtanPatternFlowDocument::Validate(
				m_FlowDocument.Get_Draft(), Build_AdmittedPatternIds(),
				FlowValidationStatus))
		{
			m_strFlowStatus =
				"Flow inventory conflict after graph reload: " +
				FlowValidationStatus;
		}
	}

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

	m_strStatus =
		"Boss graph reloaded with the canonical 28-pattern All Effects inventory.";
	return true;
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
		CNetworkManager::Get().Is_CurrentPresentationBaselineIntact(
			m_strPresentationFreshnessStatus);
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
	if (!CValtanPatternAuditionService::Get().Submit(
			CONSUMER_ID,
			BOSS_PLACEMENT_ID,
			m_strSelectedPatternId,
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

bool_t Client::CBossTool::Preview_SelectedFlowSlotIsolated()
{
	const VALTAN_PATTERN_FLOW_SLOT* pSlot = Find_SelectedFlowSlot();
	if (nullptr == pSlot || nullptr == Find_AuditionPattern(pSlot->strPatternId))
	{
		m_strFlowStatus = "Select a valid saved Flow slot first.";
		return false;
	}
	if (CValtanPatternFlowService::Get().Get_Snapshot().Is_InFlight())
	{
		m_strFlowStatus =
			"Isolated preview is unavailable while Server Flow playback is active.";
		return false;
	}

	m_bRepeat = false;
	m_strRepeatPatternId.clear();
	std::string Status;
	if (!CValtanPatternAuditionService::Get().Submit(
			FLOW_PREVIEW_CONSUMER_ID,
			BOSS_PLACEMENT_ID,
			pSlot->strPatternId,
			Status))
	{
		m_strFlowStatus = Status;
		return false;
	}
	m_strFlowStatus = Status;
	return true;
}

bool_t Client::CBossTool::Start_Flow(const bool_t bFromSelectedSlot)
{
	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow || pFlow->Slots.empty())
	{
		m_strFlowStatus = "Save at least one Flow slot before playback.";
		return false;
	}
	if (m_FlowDocument.Is_Dirty() ||
		m_FlowDocument.Has_ExternalConflict())
	{
		m_strFlowStatus =
			"Save a clean Flow revision before starting Server playback.";
		return false;
	}
	std::string ValidationStatus;
	if (!CValtanPatternFlowDocument::Validate(
			m_FlowDocument.Get_Draft(), Build_AdmittedPatternIds(),
			ValidationStatus))
	{
		m_strFlowStatus =
			"Flow no longer matches the All Effects inventory: " +
			ValidationStatus;
		return false;
	}
	if (!m_FlowDocument.Verify_SourceRevision(ValidationStatus))
	{
		m_strFlowStatus = "Flow start blocked: " + ValidationStatus;
		return false;
	}
	const std::string StartSlotId = bFromSelectedSlot ?
		m_strSelectedFlowSlotId : pFlow->Slots.front().strSlotId;
	if (StartSlotId.empty())
	{
		m_strFlowStatus = "Select a Flow slot to use Start Here.";
		return false;
	}
	if (CValtanPatternAuditionService::Get().Get_Snapshot().Is_InFlight())
	{
		m_strFlowStatus =
			"Wait for the isolated Pattern audition to finish before starting Flow.";
		return false;
	}

	m_bRepeat = false;
	m_strRepeatPatternId.clear();
	std::string Status;
	if (!CValtanPatternFlowService::Get().Start(
			BOSS_PLACEMENT_ID,
			*pFlow,
			m_FlowDocument.Get_SourceRevision(),
			StartSlotId,
			Status))
	{
		m_strFlowStatus = Status;
		return false;
	}
	m_strFlowStatus = Status;
	return true;
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
	std::string Status;
	if (!m_FlowDocument.Reload(Build_AdmittedPatternIds(), Status))
	{
		m_strFlowStatus = "Flow reload failed: " + Status;
		return false;
	}
	m_bConfirmDiscardDirtyFlow = false;
	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	if (nullptr == pFlow || pFlow->Slots.end() == std::find_if(
			pFlow->Slots.begin(), pFlow->Slots.end(),
			[this](const VALTAN_PATTERN_FLOW_SLOT& Slot)
			{
				return Slot.strSlotId == m_strSelectedFlowSlotId;
			}))
	{
		m_strSelectedFlowSlotId =
			nullptr != pFlow && !pFlow->Slots.empty() ?
				pFlow->Slots.front().strSlotId : std::string{};
	}
	m_strFlowStatus = Status;
	return true;
}

bool_t Client::CBossTool::Save_FlowDocument()
{
	std::string Status;
	if (!m_FlowDocument.Save(Build_AdmittedPatternIds(), Status))
	{
		m_strFlowStatus = "Flow save failed: " + Status;
		return false;
	}
	m_bConfirmDiscardDirtyFlow = false;
	m_strFlowStatus = Status;
	return true;
}

void Client::CBossTool::Synchronize_LiveSelection()
{
	if (!m_bFollowLive)
		return;
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!Boss.isValid || Boss.strPatternId.empty())
		return;
	const VALTAN_PATTERN_VIEW* pPattern =
		Find_AuditionPattern(Boss.strPatternId);
	if (nullptr == pPattern)
		return;
	const VALTAN_STAGE_VIEW* pStage = Find_LiveStage(*pPattern);
	m_strSelectedPatternId = pPattern->strPatternId;
	if (nullptr != pStage)
		m_strSelectedStageId = pStage->strStageId;
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
		"Server pattern verification and saved ordered Flow playback.");
	if (ImGui::BeginTabBar("##bossToolTabs"))
	{
		if (ImGui::BeginTabItem("Boss Verification"))
		{
			Render_BossVerificationTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Pattern Flow"))
		{
			Render_PatternFlowTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

void Client::CBossTool::Render_BossVerificationTab()
{
	Render_LiveSummary();
	Render_ActionBar();
	ImGui::Separator();

	if (!m_bGraphReady)
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (ImGui::Button("Retry Graph Load"))
			(void)Reload_Graph();
		return;
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
		Render_PatternList();
		ImGui::TableSetColumnIndex(1);
		Render_SelectedPattern();
		ImGui::EndTable();
	}
}

void Client::CBossTool::Render_PatternFlowTab()
{
	ImGui::TextDisabled(
		"Edit a Debug-only order. Product scriptedSequence is not published here.");
	if (!m_bGraphReady)
	{
		ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (ImGui::Button("Retry Graph Load##flow"))
			(void)Reload_Graph();
		return;
	}

	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const bool_t bPlaybackLocked = Playback.Is_InFlight();
	const char_t* pDocumentState = !m_FlowDocument.Is_Ready() ?
		"NOT LOADED" :
		(m_FlowDocument.Has_ExternalConflict() ? "EXTERNAL CONFLICT" :
			(m_FlowDocument.Is_Dirty() ? "UNSAVED" : "SAVED"));
	ImGui::Text("Flow: %s", pDocumentState);
	if (m_FlowDocument.Is_Ready() &&
		ImGui::IsItemHovered() && !m_FlowDocument.Get_SourceRevision().empty())
	{
		ImGui::SetTooltip(
			"Source SHA-256: %s",
			m_FlowDocument.Get_SourceRevision().c_str());
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(bPlaybackLocked);
	if (ImGui::Button("Reload Flow"))
	{
		if (m_FlowDocument.Is_Dirty())
		{
			m_bConfirmDiscardDirtyFlow = true;
			m_strFlowStatus =
				"Reload would discard the current draft. Confirm below.";
		}
		else
		{
			(void)Reload_FlowDocument();
		}
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!m_FlowDocument.Is_Ready() || !m_FlowDocument.Is_Dirty());
	if (ImGui::Button("Save Flow"))
		(void)Save_FlowDocument();
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	if (m_bConfirmDiscardDirtyFlow)
	{
		ImGui::TextWrapped(
			"Discard unsaved slot changes and reload the current disk revision?");
		ImGui::BeginDisabled(bPlaybackLocked);
		if (ImGui::Button("Discard & Reload"))
			(void)Reload_FlowDocument();
		ImGui::SameLine();
		if (ImGui::Button("Keep Draft"))
		{
			m_bConfirmDiscardDirtyFlow = false;
			m_strFlowStatus = "Kept the current unsaved Flow draft.";
		}
		ImGui::EndDisabled();
	}

	if (!m_FlowDocument.Is_Ready())
	{
		ImGui::TextWrapped("%s", m_strFlowStatus.c_str());
		return;
	}

	if (ImGui::BeginTable(
			"##bossPatternFlowLayout",
			2,
			ImGuiTableFlags_Resizable |
			ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn(
			"Ordered Slots", ImGuiTableColumnFlags_WidthFixed, 430.f);
		ImGui::TableSetupColumn(
			"Selected Slot", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_FlowSlotList();
		ImGui::TableSetColumnIndex(1);
		Render_FlowSelectedSlot();
		ImGui::EndTable();
	}
}

void Client::CBossTool::Render_FlowSlotList()
{
	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const bool_t bPlaybackLocked = Playback.Is_InFlight();
	ImGui::TextUnformatted("Ordered Slots");
	ImGui::SameLine();
	ImGui::BeginDisabled(bPlaybackLocked);
	if (ImGui::Button("Add from All Effects..."))
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
			std::snprintf(
				Label, sizeof(Label), "%02zu  %s%s",
				i + 1u, Name.c_str(),
				Playback.strCurrentSlotId == Slot.strSlotId ? "  [LIVE]" : "");
			ImGui::PushID(Slot.strSlotId.c_str());
			if (ImGui::Selectable(
					Label, m_strSelectedFlowSlotId == Slot.strSlotId))
			{
				m_strSelectedFlowSlotId = Slot.strSlotId;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"%s\nPattern: %s", Slot.strSlotId.c_str(),
					Slot.strPatternId.c_str());
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
	const bool_t bCanMoveUp = bHasSelection && SelectedAt != pFlow->Slots.begin();
	const bool_t bCanMoveDown = bHasSelection &&
		std::next(SelectedAt) != pFlow->Slots.end();
	ImGui::BeginDisabled(bPlaybackLocked || !bCanMoveUp);
	if (ImGui::Button("Up"))
	{
		std::string Status;
		if (!m_FlowDocument.Move_Slot(
				m_strSelectedFlowSlotId, -1, Status))
			m_strFlowStatus = Status;
		else
			m_strFlowStatus = Status;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(bPlaybackLocked || !bCanMoveDown);
	if (ImGui::Button("Down"))
	{
		std::string Status;
		if (!m_FlowDocument.Move_Slot(
				m_strSelectedFlowSlotId, 1, Status))
			m_strFlowStatus = Status;
		else
			m_strFlowStatus = Status;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(bPlaybackLocked || !bHasSelection);
	if (ImGui::Button("Remove"))
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
	const bool_t bPlaybackLocked =
		CValtanPatternFlowService::Get().Get_Snapshot().Is_InFlight();
	ImGui::BeginDisabled(bPlaybackLocked);
	ImGui::TextUnformatted("Add from All Effects");
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
					ImGui::SetTooltip("%s", PatternId.c_str());
				ImGui::PopID();
			}
		};
		ImGui::SeparatorText("CORE SERVER PATTERNS (8)");
		RenderIds(m_AuditionInventory.CorePatternIds);
		ImGui::SeparatorText("ANIMATOR PATTERNS (20)");
		RenderIds(m_AuditionInventory.AnimatorPatternIds);
	}
	ImGui::EndChild();
	ImGui::EndDisabled();
	ImGui::EndPopup();
}

void Client::CBossTool::Render_FlowSelectedSlot()
{
	const VALTAN_PATTERN_FLOW_DEFINITION* pFlow =
		m_FlowDocument.Get_DefaultFlow();
	const VALTAN_PATTERN_FLOW_SLOT* pSlot = Find_SelectedFlowSlot();
	const VALTAN_PATTERN_VIEW* pPattern = nullptr == pSlot ? nullptr :
		Find_AuditionPattern(pSlot->strPatternId);
	if (nullptr == pFlow)
		return;

	if (nullptr == pSlot || nullptr == pPattern)
	{
		ImGui::TextUnformatted("Select a Flow slot on the left.");
	}
	else
	{
		const auto SlotAt = std::find_if(
			pFlow->Slots.begin(), pFlow->Slots.end(),
			[this](const VALTAN_PATTERN_FLOW_SLOT& Slot)
			{
				return Slot.strSlotId == m_strSelectedFlowSlotId;
			});
		const size_t Ordinal = pFlow->Slots.end() == SlotAt ? 0u :
			static_cast<size_t>(std::distance(pFlow->Slots.begin(), SlotAt)) + 1u;
		ImGui::Text(
			"%02zu  %s", Ordinal,
			pPattern->strDisplayName.empty() ?
				pPattern->strPatternId.c_str() :
				pPattern->strDisplayName.c_str());
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(
				"Slot: %s\nPattern: %s",
				pSlot->strSlotId.c_str(), pSlot->strPatternId.c_str());
	}

	const VALTAN_PATTERN_FLOW_SNAPSHOT& Playback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Isolated =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t bRuntimeReady = CNetworkManager::Get().Is_Connected() &&
		Boss.isValid && Player.isValid && 0u != Player.iCurrentHp &&
		Player.isCombatReady;
	const bool_t bCanPreview = nullptr != pSlot && nullptr != pPattern &&
		bRuntimeReady && !Playback.Is_InFlight() && !Isolated.Is_InFlight();
	ImGui::BeginDisabled(!bCanPreview);
	if (ImGui::Button("Preview Isolated"))
		(void)Preview_SelectedFlowSlotIsolated();
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Uses the same Server single-pattern path as Play Selected.");

	ImGui::SeparatorText("Flow Playback");
	int32_t PursuitMs = static_cast<int32_t>(pFlow->iInterStepPursuitMs);
	ImGui::BeginDisabled(Playback.Is_InFlight());
	ImGui::SetNextItemWidth(190.f);
	if (ImGui::SliderInt(
			"Inter-step pursuit (ms)", &PursuitMs,
			100, 10000, "%d ms", ImGuiSliderFlags_AlwaysClamp))
	{
		std::string Status;
		(void)m_FlowDocument.Set_InterStepPursuitMs(
			static_cast<uint32_t>(PursuitMs), Status);
		m_strFlowStatus = Status;
	}
	ImGui::EndDisabled();
	const bool_t bSavedClean = m_FlowDocument.Is_Ready() &&
		!m_FlowDocument.Is_Dirty() &&
		!m_FlowDocument.Has_ExternalConflict();
	std::string DraftValidationStatus;
	const bool_t bDraftAdmitted = CValtanPatternFlowDocument::Validate(
		m_FlowDocument.Get_Draft(), Build_AdmittedPatternIds(),
		DraftValidationStatus);
	const bool_t bCanStart = bRuntimeReady && bSavedClean && bDraftAdmitted &&
		!pFlow->Slots.empty() && !Playback.Is_InFlight() &&
		!Isolated.Is_InFlight();
	ImGui::BeginDisabled(!bCanStart);
	if (ImGui::Button("Start First"))
		(void)Start_Flow(false);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanStart || nullptr == pSlot);
	if (ImGui::Button("Start Here"))
		(void)Start_Flow(true);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!Playback.Is_InFlight() || 0u == Playback.iRoomFlowEpoch ||
		Playback.bStopAfterCurrentRequested);
	if (ImGui::Button("Stop After Current##flow"))
	{
		std::string Status;
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
	if (VALTAN_PATTERN_FLOW_STATE::IDLE != Playback.eState)
	{
		ImGui::TextWrapped(
			"Server: %s | %s",
			Describe_ValtanPatternFlowState(Playback.eState),
			Playback.strStatus.c_str());
	}
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
			"Save a clean revision to enable Start. Isolated preview remains available.");
		if (!bDraftAdmitted)
			ImGui::TextWrapped(
				"Inventory validation: %s", DraftValidationStatus.c_str());
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
		std::string PatternText = Boss.strPatternId.empty() ?
			"IDLE" :
			(nullptr != pPattern && !pPattern->strDisplayName.empty() ?
				pPattern->strDisplayName : Boss.strPatternId);
		if (!Boss.strPatternId.empty() &&
			!m_AuditionInventory.Contains(Boss.strPatternId))
		{
			PatternText += " [live only; outside All Effects list]";
		}
		std::string StageText = nullptr == pStage ?
			(Boss.strActionId.empty() ? "IDLE" :
				"UNKNOWN ACTION " + Boss.strActionId) :
			pStage->strStageId;

		std::string TimeText = "tick " + std::to_string(Boss.iServerTick);
		if (nullptr != pStage && m_EncounterReference.Is_Ready() &&
			0u != m_EncounterReference.Get_FixedTickHz())
		{
			const uint32_t iElapsedTicks = Boss.iServerTick >= Boss.iActionStartTick ?
				Boss.iServerTick - Boss.iActionStartTick : 0u;
			const f32_t fElapsedSeconds = static_cast<f32_t>(iElapsedTicks) /
				static_cast<f32_t>(m_EncounterReference.Get_FixedTickHz());
			char_t Buffer[96]{};
			std::snprintf(
				Buffer, sizeof(Buffer), "%.2f / %.2f s",
				fElapsedSeconds,
				static_cast<f32_t>(pStage->iDurationMs) / 1000.f);
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
					"workspace changed; restart/publish");
		}
		ImGui::TextWrapped(
			"Live: %s / %s  |  %s  |  Phase %u  |  HP %u bars  |  %s",
			PatternText.c_str(), StageText.c_str(), TimeText.c_str(),
			static_cast<uint32_t>(Boss.iPhase), iHealthBars, pFreshness);
		if (nullptr != pPattern && !pPattern->strDisplayName.empty() &&
			ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Pattern ID: %s", Boss.strPatternId.c_str());
		}
	}
}

void Client::CBossTool::Render_ActionBar()
{
	const VALTAN_PATTERN_AUDITION_SNAPSHOT& Audition =
		CValtanPatternAuditionService::Get().Get_Snapshot();
	const VALTAN_PATTERN_FLOW_SNAPSHOT& FlowPlayback =
		CValtanPatternFlowService::Get().Get_Snapshot();
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	const HUD_PLAYER_STATE& Player = CCombatHUDViewModel::Get().Get_Player();
	const VALTAN_PATTERN_VIEW* pSelected =
		Find_AuditionPattern(m_strSelectedPatternId);
	const bool_t bCanPlay = m_bGraphReady &&
		nullptr != pSelected &&
		CNetworkManager::Get().Is_Connected() && Boss.isValid &&
		Player.isValid && 0u != Player.iCurrentHp && Player.isCombatReady &&
		!Audition.Is_InFlight() && !FlowPlayback.Is_InFlight();

	ImGui::TextDisabled("Replay:");
	ImGui::SameLine();
	ImGui::TextUnformatted(nullptr == pSelected ?
		"Select a pattern" :
		(pSelected->strDisplayName.empty() ?
			pSelected->strPatternId.c_str() :
			pSelected->strDisplayName.c_str()));
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanPlay);
	if (ImGui::Button("Play Selected"))
		(void)Submit_SelectedPattern();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == pSelected);
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
		if (!m_bGraphReady)
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
		else if (FlowPlayback.Is_InFlight())
			Status = "Play unavailable while ordered Pattern Flow is active.";
		else if (Audition.Is_InFlight() ||
			(CONSUMER_ID == Audition.strConsumerId &&
			 VALTAN_PATTERN_AUDITION_STATE::IDLE != Audition.eState))
			Status = Audition.strStatus;
	}
	ImGui::TextWrapped("%s", Status.c_str());
}

void Client::CBossTool::Render_PatternList()
{
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
	const auto HasVisible = [this, &Matches](
		const std::vector<std::string>& PatternIds)
	{
		return std::any_of(
			PatternIds.begin(), PatternIds.end(),
			[this, &Matches](const std::string& strPatternId)
			{
				const VALTAN_PATTERN_VIEW* pPattern =
					Find_AuditionPattern(strPatternId);
				return nullptr != pPattern && Matches(*pPattern);
			});
	};
	const auto RenderPatternIds =
		[this, &Matches, &iVisiblePatternCount](
			const std::vector<std::string>& PatternIds)
	{
		for (const std::string& strPatternId : PatternIds)
		{
			const VALTAN_PATTERN_VIEW* pPattern =
				Find_AuditionPattern(strPatternId);
			if (nullptr == pPattern || !Matches(*pPattern))
				continue;
			++iVisiblePatternCount;

			ImGui::PushID(pPattern->strPatternId.c_str());
			if (ImGui::Selectable(
					pPattern->strDisplayName.empty() ?
						pPattern->strPatternId.c_str() :
						pPattern->strDisplayName.c_str(),
					m_strSelectedPatternId == pPattern->strPatternId))
			{
				const bool_t bSelectionChanged =
					m_strSelectedPatternId != pPattern->strPatternId;
				m_bReviveFeedbackPending = false;
				if (bSelectionChanged &&
					(m_bRepeat || !m_strRepeatPatternId.empty()))
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
				m_strSelectedPatternId = pPattern->strPatternId;
				m_strSelectedStageId = pPattern->Stages.empty() ?
					std::string{} : pPattern->Stages.front().strStageId;
				m_bFollowLive = false;
			}
			if (ImGui::IsItemHovered())
			{
				if (pPattern->strDisplayName.empty())
					ImGui::SetTooltip("%s", pPattern->strPatternId.c_str());
				else
					ImGui::SetTooltip("%s\n%s",
						pPattern->strDisplayName.c_str(),
						pPattern->strPatternId.c_str());
			}
			ImGui::PopID();
		}
	};
	if (HasVisible(m_AuditionInventory.CorePatternIds))
	{
		ImGui::SeparatorText("CORE SERVER PATTERNS (8)");
		RenderPatternIds(m_AuditionInventory.CorePatternIds);
	}
	if (HasVisible(m_AuditionInventory.AnimatorPatternIds))
	{
		ImGui::SeparatorText("ANIMATOR PATTERNS (20)");
		RenderPatternIds(m_AuditionInventory.AnimatorPatternIds);
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
			"Phase %u-%u | Server-selected pattern",
			pPattern->iMinimumPhase,
			pPattern->iMaximumPhase);
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
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::BeginCombo("##bossStage", Preview.c_str()))
	{
		for (const VALTAN_STAGE_VIEW& Stage : pPattern->Stages)
		{
			ImGui::PushID(Stage.strStageId.c_str());
			const bool_t bSelected =
				Stage.strStageId == m_strSelectedStageId;
			const std::string Label =
				Stage.strStageId + " / " + Stage.strActionId;
			if (ImGui::Selectable(Label.c_str(), bSelected))
			{
				m_strSelectedStageId = Stage.strStageId;
				m_bFollowLive = false;
			}
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
	Render_ConnectionSummary(*pPattern, *pStage);
	Render_AdvancedDiagnostics(*pPattern, *pStage);
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

void Client::CBossTool::Render_AdvancedDiagnostics(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	if (!ImGui::CollapsingHeader("Why / Advanced diagnostics"))
		return;

	if (ImGui::Button("Reload Canonical Graph"))
	{
		(void)Reload_Graph();
		return;
	}
	ImGui::TextDisabled("%s", m_strCameraStatus.c_str());
	ImGui::Text(
		"Pattern: %s | Stage: %s | action: %s | kind: %s | duration: %u ms",
		Pattern.strPatternId.c_str(),
		Stage.strStageId.c_str(), Stage.strActionId.c_str(),
		Stage.strStageKind.c_str(), Stage.iDurationMs);
	ImGui::Text("Animation end: %s | suppress: %s",
		Stage.strAnimationEndPolicy.c_str(),
		Stage.bSuppressAnimation ? "true" : "false");
	const HUD_BOSS_STATE& LiveBoss = CCombatHUDViewModel::Get().Get_Boss();
	if (LiveBoss.PinnedDefinitionRevision.Is_Valid())
	{
		ImGui::TextWrapped(
			"Live pinned revision: %s (%s) | workspace: %s",
			LostArk::Shared::Format_GameplayDataRevision(
				LiveBoss.PinnedDefinitionRevision).c_str(),
			CNetworkManager::Get().Is_PresentationRevisionAvailable(
				LiveBoss.PinnedDefinitionRevision) ? "available" : "unavailable",
			m_strPresentationFreshnessStatus.c_str());
	}

	if (ImGui::TreeNode("Animation occurrences"))
	{
		if (Stage.ClipOccurrences.empty())
			ImGui::TextDisabled("None");
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
			Stage.ClipOccurrences)
		{
			ImGui::BulletText(
				"%s | %s | source %u ms | play %u ms | %.2fx",
				Clip.strClipOccurrenceId.c_str(), Clip.strClipName.c_str(),
				Clip.iSourceStartMs, Clip.iPlayMs, Clip.fPlayRate);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Effect timing and resources"))
	{
		std::map<std::string, std::filesystem::path, std::less<>> Effects;
		for (const VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
			Effects[Effect.strEffectAssetId] = Effect.DocumentPath;
		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
		{
			Effects.try_emplace(Cue.strEffectAssetId);
			ImGui::BulletText(
				"cue %s -> %s | %s",
				Cue.strBindingId.c_str(), Cue.strEffectAssetId.c_str(),
				Cue.bUsesStageClock ? "stage clock" :
					Cue.strClipOccurrenceId.c_str());
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
			Stage.CombatObjectEffects)
		{
			Effects.try_emplace(Object.strEffectAssetId);
			ImGui::BulletText(
				"object %s -> %s -> %s",
				Object.strCombatObjectArchetypeId.c_str(),
				Object.strClientVisualId.c_str(),
				Object.strEffectAssetId.c_str());
		}
		for (const std::string& IndependentId : Stage.IndependentEffectIds)
		{
			const VALTAN_INDEPENDENT_EFFECT_VIEW* pIndependent =
				Find_IndependentEffect(IndependentId);
			if (nullptr != pIndependent)
				Effects.try_emplace(pIndependent->strEffectAssetId);
		}
		for (const auto& [EffectAssetId, Path] : Effects)
			Render_EffectDocument(EffectAssetId, Path);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Camera cue details"))
	{
		if (Stage.CameraInvocations.empty())
			ImGui::TextDisabled("None");
		for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
			Stage.CameraInvocations)
		{
			ImGui::Text(
				"%s -> %s | %s +%u ms / %u ms",
				Invocation.strCameraInvocationId.c_str(),
				Invocation.strCameraCueId.c_str(),
				Invocation.strTrigger.c_str(),
				Invocation.iStartOffsetMs,
				Invocation.iDurationMs);
			const VALTAN_CINEMATIC_CAMERA_CUE* pCue =
				Find_CameraCue(Invocation.strCameraCueId);
			if (nullptr == pCue)
			{
				ImGui::TextDisabled("Missing cinematic cue document row.");
				continue;
			}
			const bool_t bExactTuple = Is_ExactCameraInvocation(
				Pattern, Stage, Invocation, *pCue);
			ImGui::Indent();
			ImGui::Text(
				"join %s | frame %s | origin %s",
				bExactTuple ? "EXACT" : "MISMATCH",
				Camera_FrameLabel(pCue->eTrackingMode),
				Format_Vector3(pCue->vTrackingOrigin).c_str());
			ImGui::Text(
				"duration %u ms | shake %.2f / %u ms",
				pCue->iDurationMs, pCue->fShakeAmplitude,
				pCue->iShakeDurationMs);
			for (const VALTAN_CINEMATIC_CAMERA_KEYFRAME& Keyframe :
				pCue->Keyframes)
			{
				ImGui::BulletText(
					"%u ms | eye %s | look %s | FOV %.1f",
					Keyframe.iTimeMs,
					Format_Vector3(Keyframe.vEye).c_str(),
					Format_Vector3(Keyframe.vLookAt).c_str(),
					Keyframe.fFovYDegrees);
			}
			ImGui::Unindent();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Server hit, motion, and world details"))
	{
		ImGui::Text(
			"hit %s | radius %.2f/%.2f | angle %.1f | length %.2f | width %.2f",
			Stage.strHitShape.c_str(), Stage.fHitInnerRadius,
			Stage.fHitOuterRadius, Stage.fHitAngleDegrees,
			Stage.fHitLength, Stage.fHitHalfWidth);
		ImGui::Text(
			"schedule count %u | delay %u ms | interval %u ms | push %.2f m / %u ms",
			Stage.iHitCount, Stage.iHitDelayMs, Stage.iHitIntervalMs,
			Stage.fPushRangeM, Stage.iPushMs);
		for (const VALTAN_STAGE_ACTION_VIEW& Action : Stage.Actions)
		{
			ImGui::BulletText(
				"%s: %s -> %s | value %.2f | duration %u ms",
				Action.strTrigger.c_str(), Action.strKind.c_str(),
				Action.strTargetId.c_str(), Action.fValue,
				Action.iDurationMs);
		}
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			ImGui::BulletText(
				"branch %s -> %s",
				Branch.strOutcome.c_str(),
				Branch.strNextActionId.has_value() ?
					Branch.strNextActionId->c_str() : "terminal");
		}
		ImGui::TreePop();
	}

	ImGui::SeparatorText("Find resource owner");
	ImGui::SetNextItemWidth(340.f);
	const bool_t bResourceQueryChanged = ImGui::InputTextWithHint(
		"##bossResourceSearch",
		"Example: fx_e_decal_007_2.dds",
		m_ResourceSearch.data(),
		m_ResourceSearch.size());
	if (bResourceQueryChanged)
	{
		m_ResourceOwnerResults.clear();
		m_ResourceSearchDocumentGenerations.clear();
		m_iResourceSearchLoadFailureCount = 0u;
		m_iResourceSearchUnverifiedCount = 0u;
		m_dNextResourceSearchFreshnessCheckSeconds = 0.0;
		m_bResourceSearchStale = false;
		m_strDiagnosticStatus = '\0' == m_ResourceSearch.front() ?
			std::string{} : "Press Find Owner to search this resource.";
	}
	ImGui::SameLine();
	ImGui::BeginDisabled('\0' == m_ResourceSearch.front());
	if (ImGui::Button("Find Owner"))
		Search_ResourceOwners();
	ImGui::EndDisabled();
	Refresh_ResourceSearchFreshness();
	if (m_bResourceSearchStale)
	{
		ImGui::TextWrapped(
			"STALE: Effect source or next-spawn catalog changed. "
			"Press Find Owner again.");
	}
	else
	{
		if (!m_strDiagnosticStatus.empty())
			ImGui::TextWrapped("%s", m_strDiagnosticStatus.c_str());
		for (const RESOURCE_OWNER_RESULT& Result :
			m_ResourceOwnerResults)
		{
			ImGui::BulletText(
				"%s / %s -> %s -> %s [%s] -> %s  |  %s",
				Result.strPatternId.c_str(), Result.strStageId.c_str(),
				Result.strEffectAssetId.c_str(), Result.strElementId.c_str(),
				Result.strSlotId.c_str(),
				Result.strResourceAssetId.c_str(),
				Result.bNextSpawnCatalogVerified ?
					"NEXT-SPAWN MATCHED - replay required" :
					"LOCAL UNVERIFIED");
		}
	}
}

void Client::CBossTool::Render_EffectDocument(
	const std::string& strEffectAssetId,
	const std::filesystem::path& PreferredPath)
{
	ImGui::PushID(strEffectAssetId.c_str());
	if (!ImGui::TreeNode(strEffectAssetId.c_str()))
	{
		ImGui::PopID();
		return;
	}
	EFFECT_DOCUMENT_CACHE_ENTRY& Entry =
		Load_EffectDocument(strEffectAssetId, PreferredPath);
	if (!Entry.bLoaded)
	{
		ImGui::TextWrapped("%s", Entry.strStatus.c_str());
	}
	else
	{
		if (Entry.bNextSpawnCatalogEquivalent)
			ImGui::TextDisabled(
				"%s", Entry.strNextSpawnCatalogStatus.c_str());
		else
			ImGui::TextWrapped(
				"UNVERIFIED: %s",
				Entry.strNextSpawnCatalogStatus.c_str());
		for (const EFFECT_ELEMENT_DESC& Element : Entry.Document.Elements)
		{
			ImGui::BulletText(
				"%s | %s | %s | %s",
				Element.strElementId.c_str(),
				Element.strDisplayName.c_str(),
				CEffectDocumentCodec::To_Token(Element.eKind),
				Element.bVisible ? "visible" : "hidden");
			ImGui::Indent();
			for (const EFFECT_RESOURCE_BINDING_DESC& Resource :
				Element.ResourceBindings)
			{
				ImGui::TextWrapped(
					"%s -> %s",
					Resource.strSlotId.c_str(),
					Resource.strAssetId.c_str());
			}
			for (const std::string& Resource : Element.UnboundSourceResources)
				ImGui::TextWrapped("unbound -> %s", Resource.c_str());
			ImGui::Unindent();
		}
	}
	ImGui::TreePop();
	ImGui::PopID();
}

void Client::CBossTool::Search_ResourceOwners()
{
	m_ResourceOwnerResults.clear();
	m_ResourceSearchDocumentGenerations.clear();
	m_iResourceSearchLoadFailureCount = 0u;
	m_iResourceSearchUnverifiedCount = 0u;
	m_dNextResourceSearchFreshnessCheckSeconds = 0.0;
	m_bResourceSearchStale = false;
	const std::string Query = m_ResourceSearch.data();
	if (Query.empty())
		return;

	std::set<std::string, std::less<>> UniqueResults;
	std::set<std::string, std::less<>> FailedDocuments;
	std::set<std::string, std::less<>> UnverifiedDocuments;
	const auto SearchPatterns =
		[this, &Query, &UniqueResults, &FailedDocuments,
		 &UnverifiedDocuments](
			const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Patterns)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			{
				std::map<std::string, std::filesystem::path, std::less<>> Effects;
				for (const VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
					Effects[Effect.strEffectAssetId] = Effect.DocumentPath;
				for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
					Effects.try_emplace(Cue.strEffectAssetId);
				for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
					Stage.CombatObjectEffects)
					Effects.try_emplace(Object.strEffectAssetId);
				for (const std::string& IndependentId : Stage.IndependentEffectIds)
				{
					const VALTAN_INDEPENDENT_EFFECT_VIEW* pIndependent =
						Find_IndependentEffect(IndependentId);
					if (nullptr != pIndependent)
						Effects.try_emplace(pIndependent->strEffectAssetId);
				}

				for (const auto& [EffectAssetId, Path] : Effects)
				{
					EFFECT_DOCUMENT_CACHE_ENTRY& Entry =
						Load_EffectDocument(EffectAssetId, Path);
					m_ResourceSearchDocumentGenerations[EffectAssetId] =
						Entry.iGeneration;
					if (!Entry.bLoaded)
					{
						FailedDocuments.insert(EffectAssetId);
						continue;
					}
					for (const EFFECT_ELEMENT_DESC& Element :
						Entry.Document.Elements)
					{
						const auto AddResult = [this, &Query, &UniqueResults,
							&UnverifiedDocuments, &Pattern, &Stage,
							&EffectAssetId, &Element, &Entry](
								const std::string& strSlotId,
								const std::string& strAssetId)
						{
							if (!Contains_CaseInsensitive(strAssetId, Query))
								return;
							const std::string Key =
								Pattern.strPatternId + "\n" + Stage.strStageId +
								"\n" + EffectAssetId + "\n" +
								Element.strElementId + "\n" + strSlotId +
								"\n" + strAssetId;
							if (!UniqueResults.insert(Key).second)
								return;
							m_ResourceOwnerResults.push_back({
								Pattern.strPatternId,
								Stage.strStageId,
								EffectAssetId,
								Element.strElementId,
								strSlotId,
								strAssetId,
								Entry.bNextSpawnCatalogEquivalent });
							if (!Entry.bNextSpawnCatalogEquivalent)
								UnverifiedDocuments.insert(EffectAssetId);
						};
						for (const EFFECT_RESOURCE_BINDING_DESC& Resource :
							Element.ResourceBindings)
						{
							AddResult(Resource.strSlotId, Resource.strAssetId);
						}
						for (const std::string& Resource :
							Element.UnboundSourceResources)
						{
							AddResult("unbound", Resource);
						}
					}
				}
			}
		}
	};
	SearchPatterns(m_Graph.Gimmicks);
	SearchPatterns(m_Graph.Rotation);
	m_iResourceSearchLoadFailureCount = FailedDocuments.size();
	m_iResourceSearchUnverifiedCount = UnverifiedDocuments.size();
	m_strDiagnosticStatus = m_ResourceOwnerResults.empty() ?
		"No admitted Valtan Effect owns that resource." :
		"Found " + std::to_string(m_ResourceOwnerResults.size()) +
			" resource owner occurrence(s).";
	if (0u != m_iResourceSearchLoadFailureCount)
	{
		m_strDiagnosticStatus += " " +
			std::to_string(m_iResourceSearchLoadFailureCount) +
			" Effect document(s) could not be inspected; the search is incomplete.";
	}
	if (0u != m_iResourceSearchUnverifiedCount)
	{
		m_strDiagnosticStatus += " " +
			std::to_string(m_iResourceSearchUnverifiedCount) +
			" Effect document(s) do not match a loaded next-spawn catalog "
			"document; "
			"those owner rows are local clues only.";
	}
}

void Client::CBossTool::Refresh_ResourceSearchFreshness()
{
	if (m_ResourceSearchDocumentGenerations.empty() ||
		m_bResourceSearchStale)
	{
		return;
	}
	const double dNowSeconds = ImGui::GetTime();
	if (dNowSeconds < m_dNextResourceSearchFreshnessCheckSeconds)
		return;
	m_dNextResourceSearchFreshnessCheckSeconds = dNowSeconds + 0.5;

	for (const auto& [EffectAssetId, iSearchGeneration] :
		m_ResourceSearchDocumentGenerations)
	{
		const auto Cached = m_EffectDocuments.find(EffectAssetId);
		if (m_EffectDocuments.end() == Cached)
		{
			m_bResourceSearchStale = true;
			return;
		}
		const std::filesystem::path PreferredPath = Cached->second.Path;
		EFFECT_DOCUMENT_CACHE_ENTRY& Current =
			Load_EffectDocument(EffectAssetId, PreferredPath);
		if (Current.iGeneration != iSearchGeneration)
		{
			m_bResourceSearchStale = true;
			return;
		}
	}
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

std::vector<std::string> Client::CBossTool::Build_AdmittedPatternIds() const
{
	std::vector<std::string> PatternIds;
	PatternIds.reserve(
		m_AuditionInventory.CorePatternIds.size() +
		m_AuditionInventory.AnimatorPatternIds.size());
	PatternIds.insert(
		PatternIds.end(),
		m_AuditionInventory.CorePatternIds.begin(),
		m_AuditionInventory.CorePatternIds.end());
	PatternIds.insert(
		PatternIds.end(),
		m_AuditionInventory.AnimatorPatternIds.begin(),
		m_AuditionInventory.AnimatorPatternIds.end());
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

std::filesystem::path Client::CBossTool::Resolve_EffectDocumentPath(
	const std::string& strEffectAssetId,
	const std::filesystem::path& PreferredPath) const
{
	if (!PreferredPath.empty())
		return PreferredPath.is_absolute() ? PreferredPath :
			CProjectDataRoot::Resolve(PreferredPath);
	return CProjectDataRoot::Resolve(
		std::filesystem::path("Effects") / "Authored" /
			(strEffectAssetId + ".effect.json"));
}

Client::CBossTool::EFFECT_DOCUMENT_CACHE_ENTRY&
Client::CBossTool::Load_EffectDocument(
	const std::string& strEffectAssetId,
	const std::filesystem::path& PreferredPath)
{
	EFFECT_DOCUMENT_CACHE_ENTRY& Entry =
		m_EffectDocuments[strEffectAssetId];
	const std::filesystem::path ResolvedPath =
		Resolve_EffectDocumentPath(
			strEffectAssetId, PreferredPath).lexically_normal();
	std::error_code FileError;
	const std::filesystem::file_time_type LastWriteTime =
		std::filesystem::last_write_time(ResolvedPath, FileError);
	const bool_t bHasLastWriteTime = !FileError;
	const bool_t bSourceChanged =
		!Entry.bLoadAttempted || Entry.Path != ResolvedPath ||
		Entry.bHasLastWriteTime != bHasLastWriteTime ||
		(bHasLastWriteTime && Entry.LastWriteTime != LastWriteTime);

	if (bSourceChanged)
	{
		Entry.bLoadAttempted = true;
		Entry.bLoaded = false;
		Entry.bNextSpawnCatalogEquivalent = false;
		Entry.Path = ResolvedPath;
		Entry.bHasLastWriteTime = bHasLastWriteTime;
		Entry.LastWriteTime = bHasLastWriteTime ?
			LastWriteTime : std::filesystem::file_time_type{};
		Entry.Document = {};
		Entry.strStatus.clear();
		Entry.strNextSpawnCatalogStatus.clear();
		if (!CEffectDocumentCodec::Load(
				Entry.Path, Entry.Document, Entry.strStatus))
		{
			Entry.strStatus = "Effect document failed: " + Entry.strStatus;
		}
		else if (Entry.Document.strEffectAssetId != strEffectAssetId)
		{
			Entry.strStatus =
				"Effect document identity mismatch: expected " +
				strEffectAssetId + ", found " +
				Entry.Document.strEffectAssetId + ".";
			Entry.Document = {};
		}
		else
		{
			Entry.bLoaded = true;
			Entry.strStatus = "Loaded.";
		}
	}

	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> NextSpawnCatalogDocument =
		CEffectCatalog::Find_Loaded(strEffectAssetId);
	const bool_t bNextSpawnCatalogChanged =
		NextSpawnCatalogDocument.get() !=
			Entry.pNextSpawnCatalogDocument.get();
	if (bSourceChanged || bNextSpawnCatalogChanged)
	{
		Entry.iGeneration =
			(std::numeric_limits<size_t>::max)() == Entry.iGeneration ?
				1u : Entry.iGeneration + 1u;
		Entry.pNextSpawnCatalogDocument = NextSpawnCatalogDocument;
		Entry.bNextSpawnCatalogEquivalent = false;
		if (!Entry.bLoaded)
		{
			Entry.strNextSpawnCatalogStatus =
				"Current authored Effect document could not be compared to "
				"the next-spawn catalog.";
		}
		else if (nullptr == NextSpawnCatalogDocument)
		{
			Entry.strNextSpawnCatalogStatus =
				"Next-spawn Effect catalog document is not loaded. Play its "
				"owner pattern or prewarm it before treating these local owner "
				"rows as evidence.";
		}
		else if (CEffectDocumentCodec::Serialize(Entry.Document) ==
			CEffectDocumentCodec::Serialize(*NextSpawnCatalogDocument))
		{
			Entry.bNextSpawnCatalogEquivalent = true;
			Entry.strNextSpawnCatalogStatus =
				"Next-spawn Effect catalog matches current authored source. "
				"Replay is required; the active occurrence remains unverified.";
		}
		else
		{
			Entry.strNextSpawnCatalogStatus =
				"Next-spawn Effect catalog is stale or different. Owner data is "
				"unverified; save a hot replacement or restart before judging it.";
		}
	}
	return Entry;
}
