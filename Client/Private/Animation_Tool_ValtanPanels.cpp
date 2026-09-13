#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "BalanceTool.h"
#include "ValtanBossTool.h"
#include "Character.h"
#include "MainApp.h"
#include "Model.h"
#include "SoundCueCatalog.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>




void Client::CAnimation_Tool::Render_ValtanPatternMasterUnavailableShell(
	const std::size_t iAdmittedPatternCount,
	const bool_t bHasPreviewModel)
{
	if (ImGui::BeginTable(
		"##ValtanWorkbenchMain", 3,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV |
			ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollX,
		ImVec2(0.f, 420.f), WORKBENCH_THREE_PANE_INNER_WIDTH))
	{
		ImGui::TableSetupColumn(
			"Master / Outliner", ImGuiTableColumnFlags_WidthStretch, 0.22f);
		ImGui::TableSetupColumn(
			"Preview / Transport", ImGuiTableColumnFlags_WidthStretch, 0.46f);
		ImGui::TableSetupColumn(
			"Persistent Detail", ImGuiTableColumnFlags_WidthStretch, 0.32f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if (ImGui::BeginChild(
			"##ValtanWorkbenchOutliner", ImVec2(0.f, 0.f),
			ImGuiChildFlags_Borders))
		{
			ImGui::SeparatorText("Master / Outliner");
			ImGui::TextUnformatted("Valtan | boss.valtan");
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.35f, 1.f),
				"Pattern data unavailable");
			ImGui::TextWrapped(
				"%zu editable Pattern rows remain in the previous read-only view.",
				iAdmittedPatternCount);
		}
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);
		if (ImGui::BeginChild(
			"##ValtanWorkbenchPreview", ImVec2(0.f, 0.f),
			ImGuiChildFlags_Borders))
		{
			ImGui::SeparatorText("Preview / Transport");
			ImGui::TextDisabled(
				"Preview Model: %s", bHasPreviewModel ? "READY" : "MISSING");
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.35f, 1.f),
				"Complete Play is unavailable until Pattern data loads.");
			ImGui::TextWrapped(
				"Fix the Load error below, then Reload Valtan Pattern Master. Failed loads preserve the previous view without partially replacing it.");
		}
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(2);
		if (ImGui::BeginChild(
			"##ValtanWorkbenchDetail", ImVec2(0.f, 0.f),
			ImGuiChildFlags_Borders))
		{
			ImGui::SeparatorText("Persistent Detail");
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.35f, 1.f),
				"Pattern Data: %s",
				ValtanPatternMasterAdmissionLabel());
			if (!m_strValtanPatternMasterStatus.empty())
				ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());
		}
		ImGui::EndChild();
		ImGui::EndTable();
	}

	if (ImGui::BeginChild(
		"##ValtanWorkbenchSequencer", ImVec2(0.f, 180.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Sequencer / Joined Tracks");
		ImGui::TextDisabled(
			"Animation, Effect, Sound, Camera, Light/World and Combat Object tracks require one loaded stable Pattern/Stage selection.");
	}
	ImGui::EndChild();

	if (ImGui::BeginChild(
		"##ValtanWorkbenchDataFiles", ImVec2(0.f, 390.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Data Files / Load Status");
		ImGui::TextWrapped(
			"%s", m_strValtanPatternMasterStatus.empty() ?
				"Load returned no additional detail." :
				m_strValtanPatternMasterStatus.c_str());
		ImGui::Separator();
		ImGui::BulletText("Gameplay | Data/Valtan/Valtan.gameplay.json");
		ImGui::BulletText("Presentation | Data/Valtan/Valtan.presentation.json");
		ImGui::BulletText("Encounter | Data/Encounters/Valtan/ValtanEncounter.json");
		ImGui::BulletText(
			"Animation Product (read-only) | Data/Animation/Authored/Valtan/Valtan.patternbindings.json");
		ImGui::BulletText(
			"Effect Invocation Product (read-only) | Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json");
		ImGui::BulletText(
			"Sound | Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
		ImGui::BulletText(
			"Camera/World | Data/Encounters/Valtan typed owner documents");
		ImGui::BeginDisabled(nullptr == m_pBalanceTool);
		if (ImGui::SmallButton("Open Valtan Balance / Gameplay##RejectedDataFiles"))
			m_pBalanceTool->Open_Valtan();
		ImGui::EndDisabled();
	}
	ImGui::EndChild();
}

void Client::CAnimation_Tool::Render_ValtanPatternMaster(
	const shared_ptr<Engine::CModel>& pModel)
{
	const bool_t bHasPreviewModel = nullptr != pModel;
	bool_t bReloadPatternMasterAfterSave = false;
	if (!m_bValtanPatternMasterLoadAttempted)
	{
		m_bValtanPatternMasterLoadAttempted = true;
		(void)Reload_ValtanPatternMaster();
	}
	const bool_t bMutationAdmitted =
		Can_MutateValtanView(m_eValtanPatternMasterAdmission);
	ImGui::SeparatorText("Valtan Action Presentation Workbench");
	ImGui::TextWrapped(
		"One joined view over Server Stage, Animation, Effect, Sound Asset, and Combat Object. Pattern Offline samples the Product animation locally; Server Replay/Live submits the same stable pattern ID to the real Arena authority, where movement, hit, grab, damage, Effect, and Sound run.");
	ImGui::TextDisabled(
		"Encounter target: Valtan (Product Server authority). KoukuSaydon Arena is admitted; its boss/pattern Server vertical slice remains deferred.");
	ImGui::SeparatorText("Applied Product Sources / Editability");
	ImGui::BulletText(
		"EDIT + SAVE: Data/Valtan/Valtan.gameplay.json | Server stage clock, collider, hit schedule and player reaction");
	ImGui::BulletText(
		"EDIT + SAVE: Data/Balance/BossProfiles.json and DamageProfiles.json | Valtan gameplay values through the typed Balance transaction");
	ImGui::BulletText(
		"AUTHORING OWNER: Data/Valtan/Valtan.presentation.json | animation occurrences and managed Effect invocations save only through the immutable joined revision pipeline");
	ImGui::BulletText(
		"EDIT + OWNER SAVE: Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json | stable-row Sound event, derived bank, source start and repeat policy");
	ImGui::BulletText(
		"Generated files (read-only): Valtan.patternbindings.json + Valtan.patterneffectcues.json | use Save and Load through their owning editors");
	ImGui::BulletText(
		"JOINED LANES: V1 Effect, Sound, Camera/Shake, World Event and Server Combat Object bindings are listed below with their owner-tool boundary");
	ImGui::TextDisabled(
		"Animation Sequence Intake is an offline reviewed source. It is not presented as Product Save until Client presentation generation staging and rollback are admitted.");
	if (!bMutationAdmitted)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.55f, 0.25f, 1.f),
			"READ-ONLY: Pattern data is %s. Save, Create, Complete Play and Server actions stay disabled until Load succeeds.",
			ValtanPatternMasterAdmissionLabel());
	}
	ImGui::BeginDisabled(nullptr == m_pBalanceTool);
	if (ImGui::SmallButton("Open Valtan Balance / Gameplay"))
		m_pBalanceTool->Open_Valtan();
	ImGui::EndDisabled();

	if (ImGui::CollapsingHeader("Server Arena Environment"))
	{
	ImGui::TextDisabled(
		"These presets use the active Valtan room's destruction transaction; no wall, collision or navigation state is changed locally.");
	ImGui::PushID("WorkbenchArenaPreset");
	ImGui::BeginDisabled(nullptr == m_pValtanBossTool || !bMutationAdmitted);
	const auto arenaPresetButton = [this](
		const char_t* label,
		const LostArk::Shared::VALTAN_ARENA_PRESET preset)
	{
		if (!ImGui::Button(label))
			return;
		std::string status;
		(void)m_pValtanBossTool->Set_ServerArenaPreset(preset, status);
		m_strValtanPatternMasterStatus = std::move(status);
	};
	arenaPresetButton(
		"Fresh / All Walls", LostArk::Shared::VALTAN_ARENA_PRESET::FRESH);
	ImGui::SameLine();
	arenaPresetButton(
		"Circle / Walls Gone",
		LostArk::Shared::VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE);
	arenaPresetButton(
		"Break 3 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN);
	ImGui::SameLine();
	arenaPresetButton(
		"Break 9 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN);
	arenaPresetButton(
		"Break 3 + 9 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN);
	ImGui::EndDisabled();

	CValtanBossTool::VALTAN_ARENA_ACTIVE_STATE activeState{};
	std::string activeStateStatus;
	const bool_t bActiveStateReady = nullptr != m_pValtanBossTool &&
		m_pValtanBossTool->Get_ServerArenaActiveState(
			activeState, activeStateStatus);
	ImGui::SeparatorText("Arena Active (Server actual)");
	ImGui::TextDisabled(
		"Active boxes are replicated facts. Mutations use the five exact Server presets above.");
	const auto actualCheckbox = [](const char_t* label, const bool_t active)
	{
		bool_t value = active;
		ImGui::BeginDisabled(true);
		ImGui::Checkbox(label, &value);
		ImGui::EndDisabled();
	};
	ImGui::BeginDisabled(!bActiveStateReady);
	actualCheckbox(
		"Ordinary walls / debris sources Active",
		activeState.bOrdinaryWallsActive);
	actualCheckbox("109 outer ring Active", activeState.bOuterRingActive);
	actualCheckbox(
		"3 o'clock floor / collision / Nav Active",
		activeState.bThreeOClockFloorActive);
	actualCheckbox(
		"9 o'clock floor / collision / Nav Active",
		activeState.bNineOClockFloorActive);
	ImGui::EndDisabled();
	ImGui::Text(
		"Debris actors %u | collision %u | nav regions %u | nav revision %llu",
		activeState.iDebrisActorCount,
		activeState.iActiveCollisionCount,
		activeState.iActiveNavigationRegionCount,
		static_cast<unsigned long long>(activeState.iNavigationRevision));
	if (!activeStateStatus.empty())
		ImGui::TextDisabled("%s", activeStateStatus.c_str());
	if (nullptr != m_pValtanBossTool)
	{
		const std::string arenaStatus =
			m_pValtanBossTool->Get_ServerArenaPresetStatus();
		if (!arenaStatus.empty())
			ImGui::TextWrapped("Arena: %s", arenaStatus.c_str());
	}
	ImGui::PopID();
	}

	if (!m_bValtanPatternMasterLoadAttempted)
	{
		m_bValtanPatternMasterLoadAttempted = true;
		Reload_ValtanPatternMaster();
	}
	ImGui::BeginDisabled(
		m_bValtanPatternMasterPlaying || m_bValtanPatternPreviewPlaying ||
		m_bValtanPatternSoundCuesDirty ||
		m_bValtanCombatObjectSoundCuesDirty ||
		m_bValtanPatternAnimationBindingDirty);
	if (ImGui::SmallButton("Reload Valtan Pattern Master"))
		Reload_ValtanPatternMaster();
	ImGui::EndDisabled();
	if (m_bValtanPatternSoundCuesDirty && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Save or discard the typed Pattern Sound draft before reload.");
	}
	else if (m_bValtanCombatObjectSoundCuesDirty && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("Save the edited Server-hit Sound binding before reload.");
	}
	else if (m_bValtanPatternAnimationBindingDirty && ImGui::IsItemHovered(
		ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Save or discard the typed Animation binding draft before reload.");
	}

	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_ValtanPatternMasterPatterns();
	const bool_t bHasLoadedPatterns = !Patterns.empty();
	const bool_t bReady = bHasLoadedPatterns &&
		Can_DisplayValtanView(m_eValtanPatternMasterAdmission);
	ImGui::SeparatorText("Workbench Data");
	ImGui::Text(
		"Preview Model: %s | Pattern Data: %s | Inventory: %zu editable patterns",
		bHasPreviewModel ? "READY" : "MISSING",
		ValtanPatternMasterAdmissionLabel(),
		Patterns.size());
	if (!bReady)
	{
		if (!m_strValtanPatternMasterStatus.empty())
			ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());
		Render_ValtanPatternMasterUnavailableShell(
			Patterns.size(), bHasPreviewModel);
	}
	else
	{
	m_iValtanPatternMasterSelected = std::clamp(
		m_iValtanPatternMasterSelected, 0,
		static_cast<int32_t>(Patterns.size() - 1u));
	const VALTAN_PATTERN_VIEW* pSelected =
		Patterns[static_cast<size_t>(m_iValtanPatternMasterSelected)];
	const auto StablePattern = std::find_if(
		Patterns.begin(), Patterns.end(),
		[this](const VALTAN_PATTERN_VIEW* pPattern)
		{
			return nullptr != pPattern &&
				pPattern->strPatternId == m_strValtanWorkbenchPatternId;
		});
	if (Patterns.end() != StablePattern)
		pSelected = *StablePattern;
	else
	{
		m_strValtanWorkbenchPatternId = pSelected->strPatternId;
		m_strValtanWorkbenchStageId.clear();
	}
#ifdef _DEBUG
	if (!m_bValtanPatternSoundCuesDirty)
	{
		if (CMainApp* const pApp = CMainApp::Get_Active())
		{
			const std::string& strSharedPatternId =
				pApp->Debug_GetSelectedCompletePlayPatternId();
			if (strSharedPatternId.empty())
			{
				(void)pApp->Debug_SelectCompletePlayPattern(
					pSelected->strPatternId);
			}
			else
			{
				const auto Found = std::find_if(
					Patterns.begin(), Patterns.end(),
					[&strSharedPatternId](const VALTAN_PATTERN_VIEW* pPattern)
					{
						return nullptr != pPattern &&
							pPattern->strPatternId == strSharedPatternId;
					});
				if (Patterns.end() != Found)
				{
					if (m_strValtanWorkbenchPatternId !=
						(*Found)->strPatternId)
					{
						m_strValtanWorkbenchStageId.clear();
						m_eValtanWorkbenchSelection =
							VALTAN_WORKBENCH_SELECTION_KIND::PATTERN;
					}
					pSelected = *Found;
					m_strValtanWorkbenchPatternId = pSelected->strPatternId;
				}
			}
		}
	}
#endif
	const auto SelectedPatternIterator = std::find_if(
		Patterns.begin(), Patterns.end(),
		[pSelected](const VALTAN_PATTERN_VIEW* pPattern)
		{
			return pPattern == pSelected;
		});
	m_iValtanPatternMasterSelected = static_cast<int32_t>(
		std::distance(Patterns.begin(), SelectedPatternIterator));
	const VALTAN_STAGE_VIEW* pSelectedStage = nullptr;
	const auto SelectedStageIterator = std::find_if(
		pSelected->Stages.begin(), pSelected->Stages.end(),
		[this](const VALTAN_STAGE_VIEW& Stage)
		{
			return Stage.strStageId == m_strValtanWorkbenchStageId;
		});
	if (pSelected->Stages.end() != SelectedStageIterator)
		pSelectedStage = &*SelectedStageIterator;
	else if (!pSelected->Stages.empty())
	{
		pSelectedStage = &pSelected->Stages.front();
		m_strValtanWorkbenchStageId = pSelectedStage->strStageId;
	}

	const auto SelectWorkbenchItem =
		[this, &Patterns, &pSelected, &pSelectedStage](
			const VALTAN_PATTERN_VIEW& Pattern,
			const VALTAN_STAGE_VIEW* pStage,
			const VALTAN_WORKBENCH_SELECTION_KIND eKind)
	{
		const bool_t bPatternChanged =
			m_strValtanWorkbenchPatternId != Pattern.strPatternId;
		pSelected = &Pattern;
		pSelectedStage = pStage;
		if (nullptr == pSelectedStage && !Pattern.Stages.empty())
			pSelectedStage = &Pattern.Stages.front();
		m_strValtanWorkbenchPatternId = Pattern.strPatternId;
		m_strValtanWorkbenchStageId = nullptr == pSelectedStage ?
			std::string{} : pSelectedStage->strStageId;
		m_eValtanWorkbenchSelection = eKind;
		const auto Selected = std::find_if(
			Patterns.begin(), Patterns.end(),
			[&Pattern](const VALTAN_PATTERN_VIEW* pCandidate)
			{
				return nullptr != pCandidate &&
					pCandidate->strPatternId == Pattern.strPatternId;
			});
		if (Patterns.end() != Selected)
		{
			m_iValtanPatternMasterSelected = static_cast<int32_t>(
				std::distance(Patterns.begin(), Selected));
		}
		if (bPatternChanged)
			m_eValtanPatternMasterPath = VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
#ifdef _DEBUG
		if (CMainApp* const pApp = CMainApp::Get_Active())
			(void)pApp->Debug_SelectCompletePlayPattern(Pattern.strPatternId);
#endif
	};

	const bool_t bWorkbenchMainVisible = ImGui::BeginTable(
		"##ValtanWorkbenchMain", 3,
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_SizingStretchProp |
		ImGuiTableFlags_ScrollX,
		ImVec2(0.f, 680.f), WORKBENCH_THREE_PANE_INNER_WIDTH);
	if (!bWorkbenchMainVisible)
	{
		ImGui::TextDisabled(
			"Main panes are outside the current scroll clip; Sequencer and Data Files remain available below.");
	}
	else
	{
	ImGui::TableSetupColumn(
		"Master / Outliner", ImGuiTableColumnFlags_WidthStretch, 0.22f);
	ImGui::TableSetupColumn(
		"Preview / Transport", ImGuiTableColumnFlags_WidthStretch, 0.46f);
	ImGui::TableSetupColumn(
		"Persistent Detail", ImGuiTableColumnFlags_WidthStretch, 0.32f);
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	if (ImGui::BeginChild(
		"##ValtanWorkbenchOutliner", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Master / Outliner");
		ImGui::BeginDisabled(
			m_bValtanPatternMasterPlaying ||
			m_bValtanPatternSoundCuesDirty);
		const bool_t bTargetSelected =
			VALTAN_WORKBENCH_SELECTION_KIND::TARGET ==
				m_eValtanWorkbenchSelection;
		if (ImGui::Selectable(
			"Valtan##boss.valtan", bTargetSelected,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			m_eValtanWorkbenchSelection =
				VALTAN_WORKBENCH_SELECTION_KIND::TARGET;
		}
		ImGui::TextDisabled("boss.valtan | Product Server authority");
		for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
		{
			if (nullptr == pPattern)
				continue;
			const VALTAN_PATTERN_VIEW& Pattern = *pPattern;
			ImGui::PushID(Pattern.strPatternId.c_str());
			ImGuiTreeNodeFlags PatternFlags =
				ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth;
			if (VALTAN_WORKBENCH_SELECTION_KIND::PATTERN ==
					m_eValtanWorkbenchSelection &&
				m_strValtanWorkbenchPatternId == Pattern.strPatternId)
			{
				PatternFlags |= ImGuiTreeNodeFlags_Selected;
			}
			const std::string PatternLabel = Pattern.strDisplayName + " | " +
				Pattern.strPatternId + "##pattern-tree";
			const bool_t bPatternOpen = ImGui::TreeNodeEx(
				PatternLabel.c_str(), PatternFlags);
			if (ImGui::IsItemClicked())
			{
				SelectWorkbenchItem(
					Pattern, nullptr,
					VALTAN_WORKBENCH_SELECTION_KIND::PATTERN);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"%s | %zu stable stages",
					Pattern.strPatternId.c_str(), Pattern.Stages.size());
			}
			if (bPatternOpen)
			{
				for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				{
					ImGui::PushID(Stage.strStageId.c_str());
					const bool_t bStageSelected =
						VALTAN_WORKBENCH_SELECTION_KIND::STAGE ==
							m_eValtanWorkbenchSelection &&
						m_strValtanWorkbenchPatternId ==
							Pattern.strPatternId &&
						m_strValtanWorkbenchStageId == Stage.strStageId;
					const std::string StageLabel = Stage.strStageId +
						" | " + Stage.strSequenceRole + " | " +
						Stage.strActionId;
					if (ImGui::Selectable(
						StageLabel.c_str(), bStageSelected))
					{
						SelectWorkbenchItem(
							Pattern, &Stage,
							VALTAN_WORKBENCH_SELECTION_KIND::STAGE);
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::EndDisabled();
	}
	ImGui::EndChild();

	ImGui::TableSetColumnIndex(1);
	ImGui::BeginChild(
		"##ValtanWorkbenchPreview", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders);
	ImGui::SeparatorText("Preview / Transport");
	if (pSelected->bManualServerAudition)
	{
		ImGui::TextDisabled(
			"Manual Server audition | phase %u | source chain %s | automatic rotation disabled",
			pSelected->iAuthoringPhase,
			pSelected->strSourceAnimationChainId.c_str());
	}
	ImGui::BeginDisabled(
		m_bValtanPatternMasterPlaying ||
		m_bValtanPatternSoundCuesDirty);
	if (ImGui::BeginCombo(
		"Pattern##ValtanPatternMaster",
		(pSelected->strPatternId + " | " +
			pSelected->strDisplayName).c_str()))
	{
		for (size_t iPattern = 0u; iPattern < Patterns.size(); ++iPattern)
		{
			const VALTAN_PATTERN_VIEW& Pattern = *Patterns[iPattern];
			const std::string Label = Pattern.strPatternId + " | " +
				Pattern.strDisplayName;
			if (ImGui::Selectable(
				Label.c_str(),
				static_cast<int32_t>(iPattern) ==
					m_iValtanPatternMasterSelected))
			{
				SelectWorkbenchItem(
					Pattern, nullptr,
					VALTAN_WORKBENCH_SELECTION_KIND::PATTERN);
			}
		}
		ImGui::EndCombo();
	}
	if ("VALTAN_DASH_CHARGE" == pSelected->strPatternId &&
		ImGui::BeginCombo(
			"Dash authoring path##ValtanPatternMaster",
			ValtanPatternMasterPathName(m_eValtanPatternMasterPath)))
	{
		for (const VALTAN_PATTERN_PREVIEW_PATH eCandidate : {
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
			VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK })
		{
			if (ImGui::Selectable(
				ValtanPatternMasterPathName(eCandidate),
				eCandidate == m_eValtanPatternMasterPath))
			{
				m_eValtanPatternMasterPath = eCandidate;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	ImGui::SeparatorText("Playback Modes");
	ImGui::BeginDisabled(!bHasPreviewModel || !bMutationAdmitted);
	if (ImGui::Button("Pattern Offline"))
	{
		Start_ValtanPatternMasterPreview(
			pModel, *pSelected, m_eValtanPatternMasterPath);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bMutationAdmitted);
	if (ImGui::Button("Complete Play (Server/Arena)"))
	{
		std::string Status;
#ifdef _DEBUG
		if (CMainApp* const pApp = CMainApp::Get_Active())
		{
			if (pApp->Debug_SelectCompletePlayPattern(
					pSelected->strPatternId))
			{
				(void)pApp->Debug_CompletePlaySelected(Status);
			}
			else
			{
				Status =
					"Selected Workbench pattern is not in the shared Server inventory.";
			}
		}
		else
			Status = "Complete Play workspace is unavailable.";
#else
		Status = "Complete Play is available only in a Debug authoring build.";
#endif
		m_strValtanPatternMasterStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		!bHasPreviewModel || !m_bValtanPatternMasterPlaying);
	if (ImGui::Button(
		m_bValtanPatternMasterPaused ? "Resume Master" : "Pause Master"))
	{
		m_bValtanPatternMasterPaused = !m_bValtanPatternMasterPaused;
		pModel->Set_AnimPaused(true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop Master"))
	{
		Stop_ValtanPatternMasterPreview(
			pModel, "Valtan Pattern Master preview stopped; idle restored.");
	}
	ImGui::EndDisabled();
	if (!bHasPreviewModel)
	{
		ImGui::TextDisabled(
			"Local Pattern Offline transport requires the Valtan Model View. Complete Play remains Server-authoritative and requires ADMITTED canonical data.");
	}

	const auto RenderPersistentSave =
		[this, &bReloadPatternMasterAfterSave, bMutationAdmitted]()
	{
	ImGui::SeparatorText("Authoring Revision");
	const bool_t bDraftDirty = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Is_ValtanDraftDirty();
	ImGui::TextDisabled(
		"%s | gameplay wall clock and presentation remain separate joined domains",
		bDraftDirty ? "UNSAVED GAMEPLAY DRAFT" : "saved source");
	if (m_bValtanCombatObjectSoundCuesDirty)
		ImGui::TextColored(ImVec4(1.f, 0.75f, 0.2f, 1.f),
			"UNSAVED SERVER-HIT SOUND DRAFT (read-only until its canonical typed transaction is available)");
	ImGui::BeginDisabled(nullptr == m_pBalanceTool || !bMutationAdmitted);
	if (ImGui::Button("Save & Apply##ValtanPatternMaster"))
	{
		std::string Status;
		if (m_bValtanCombatObjectSoundCuesDirty)
		{
			Status =
				"Save blocked: combat-object Sound is part of immutable presentation M; the legacy direct source replacement was retired and this draft was not written.";
		}
		else if (m_pBalanceTool->Save_ValtanProduct(Status))
		{
			bReloadPatternMasterAfterSave = true;
		}
		m_strValtanPatternMasterStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	if (nullptr != m_pBalanceTool)
	{
		ImGui::TextDisabled("Runtime activation: %s",
			m_pBalanceTool->Get_ValtanCandidateApplyClass().empty() ? "NONE" :
				m_pBalanceTool->Get_ValtanCandidateApplyClass().c_str());
	}
	};

	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
		"Preview speed##ValtanPatternMaster",
		&m_fValtanPatternPreviewSpeed, 0.25f, 3.f, "%.2fx"))
	{
		m_fValtanPatternPreviewSpeed = std::clamp(
			m_fValtanPatternPreviewSpeed, 0.25f, 3.f);
	}
	if (bHasPreviewModel && m_bValtanPatternMasterPlaying &&
		m_iValtanPatternMasterItem < m_ValtanPatternMasterPlaylist.size())
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
		f32_t fTimelineSeconds =
			static_cast<f32_t>(Item.iTimelineStartMs) * 0.001f +
			m_fValtanPatternMasterItemElapsedSeconds;
		const f32_t fDurationSeconds =
			static_cast<f32_t>(m_iValtanPatternMasterDurationMs) * 0.001f;
		char_t TimelineFormat[96]{};
		snprintf(
			TimelineFormat, sizeof(TimelineFormat),
			"%%.3f s / %.3f s", fDurationSeconds);
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::SliderFloat(
			"##ValtanPatternMasterTimeline",
			&fTimelineSeconds, 0.f, fDurationSeconds, TimelineFormat))
		{
			if (!Seek_ValtanPatternMasterPreview(
				pModel, fTimelineSeconds, true, true))
			{
				m_strValtanPatternMasterStatus =
					"Master seek rejected; current admitted pose preserved.";
			}
		}
		if (Item.bSuppressAnimation)
		{
			ImGui::Text(
				"Now %s / %s | animation NONE | wall %u ms",
				Item.strStageId.c_str(), Item.strSequenceRole.c_str(),
				Item.iAuthoringWallMs);
			ImGui::TextDisabled(
				"Boss pose hold; no additional Valtan animation clip is started.");
		}
		else
		{
			ImGui::Text(
				"Now %s / %s | occurrence %u/%u | wall %u ms",
				Item.strStageId.c_str(), Item.strSequenceRole.c_str(),
				Item.iOccurrenceNumber, Item.iOccurrenceCount,
				Item.iAuthoringWallMs);
			ImGui::TextDisabled(
				"%s | source +%u ms, play %u ms, rate %.6g, repeatUntilStageEnd=%s",
				Item.strClipName.c_str(), Item.iSourceStartMs, Item.iPlayMs,
				Item.fPlayRate,
				Item.bRepeatUntilStageEnd ? "true" : "false");
		}
	}
	if (!m_strValtanPatternMasterStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());

	if (ImGui::CollapsingHeader("Selection / Presentation Reference"))
	{
	ImGui::SeparatorText("Weighted normal selection");
	ImGui::TextDisabled(
		"%s | %zu loaded normal patterns; health-bar mechanics keep queue precedence",
		m_ValtanPatternMasterView.NormalSelection.strSelectionMode.c_str(),
		m_ValtanPatternMasterView.NormalSelection.PatternIds.size());
	for (const VALTAN_NORMAL_SELECTION_RANGE_VIEW& Range :
		m_ValtanPatternMasterView.NormalSelection.Ranges)
	{
		ImGui::BulletText(
			"%s | bars %u > HP bar > %u",
			Range.strRotationId.c_str(), Range.iFromHealthBar,
			Range.iToHealthBar);
	}
	for (const std::string& PatternId :
		m_ValtanPatternMasterView.NormalSelection.PatternIds)
	{
		const auto Found = std::find_if(
			Patterns.begin(), Patterns.end(),
			[&PatternId](const VALTAN_PATTERN_VIEW* pPattern)
			{
				return nullptr != pPattern &&
					pPattern->strPatternId == PatternId;
			});
		if (Found == Patterns.end())
			continue;
		const VALTAN_PATTERN_VIEW& Pattern = **Found;
		ImGui::TextDisabled(
			"%s | weight %u | range %.1f..%.1f | max consecutive %u | %s / %s",
			Pattern.strPatternId.c_str(), Pattern.iSelectionWeight,
			Pattern.fMinimumRange, Pattern.fMaximumRange,
			Pattern.iMaximumConsecutiveUses,
			Pattern.strArmorRequirement.c_str(),
			Pattern.strPhaseRequirement.c_str());
	}

	ImGui::SeparatorText("Counter reaction animation layers (reference only)");
	ImGui::TextDisabled(
		"Reference-only counter reactions are separate from the editable Pattern list above.");
	for (const VALTAN_COUNTER_REACTION_LAYER_VIEW& Layer :
		m_ValtanPatternMasterView.CounterReactionLayers)
	{
		ImGui::PushID(Layer.strReactionLayerId.c_str());
		const std::string Label = Layer.strReactionLayerId + " | " +
			Layer.strOwnerPatternId + "/" + Layer.strOwnerStageId;
		if (ImGui::TreeNodeEx(Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		{
			for (const auto* pAction :
				{ &Layer.Window, &Layer.Success, &Layer.Failure })
			{
				ImGui::TextDisabled("action %s", pAction->strActionId.c_str());
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
					pAction->ClipOccurrences)
				{
					ImGui::BulletText(
						"%s | %s", Clip.strClipOccurrenceId.c_str(),
						Clip.strClipName.c_str());
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::SeparatorText("Presentation sources");
	for (const VALTAN_PRESENTATION_SOURCE_VIEW& Source :
		pSelected->PresentationSources)
	{
		ImGui::BulletText(
			"action %u | sequence %u | %s",
			Source.iSourceActionId, Source.iSequenceIndex,
			Source.strRole.c_str());
	}
	}

	ImGui::SeparatorText("Selected stage / sequence role");
	std::vector<const VALTAN_STAGE_VIEW*> PreviewPath;
	std::string PreviewPathStatus;
	if (CValtanPatternTree::Build_PreviewStagePath(
		*pSelected, m_eValtanPatternMasterPath,
		PreviewPath, PreviewPathStatus))
	{
		std::string PathLabel = "Current path: ";
		for (size_t iStage = 0u; iStage < PreviewPath.size(); ++iStage)
		{
			if (0u != iStage)
				PathLabel += " -> ";
			PathLabel += PreviewPath[iStage]->strStageId;
		}
		ImGui::TextDisabled("%s", PathLabel.c_str());
	}
	else
	{
		ImGui::TextDisabled("Current path rejected: %s",
			PreviewPathStatus.c_str());
	}
	for (size_t iStage = 0u; iStage < pSelected->Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = pSelected->Stages[iStage];
		if (nullptr == pSelectedStage ||
			Stage.strStageId != pSelectedStage->strStageId)
		{
			continue;
		}
		ImGui::PushID(static_cast<int32_t>(iStage));
		char_t Label[512]{};
		snprintf(
			Label, sizeof(Label),
			"%02zu  %s | %s | %u ms | %s | repeatCount %u | %zu occurrences",
			iStage + 1u, Stage.strStageId.c_str(),
			Stage.strSequenceRole.c_str(), Stage.iDurationMs,
			Stage.strAnimationEndPolicy.c_str(),
			Stage.iAuthoringRepeatCount, Stage.ClipOccurrences.size());
		if (ImGui::TreeNodeEx(Label, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("action %s | %s | damage %s",
				Stage.strActionId.c_str(), Stage.strStageKind.c_str(),
				Stage.strServerDamageProfileId.empty() ? "NONE" :
					Stage.strServerDamageProfileId.c_str());
			for (size_t iClip = 0u;
				iClip < Stage.ClipOccurrences.size(); ++iClip)
			{
				const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
					Stage.ClipOccurrences[iClip];
				ImGui::BulletText(
					"%zu. %s | %s | wall %u ms",
					iClip + 1u, Clip.strClipName.c_str(),
					Clip.strClipOccurrenceId.c_str(),
					Clip.iAuthoringWallMs);
				ImGui::Indent();
				ImGui::TextDisabled(
					"sourceStartMs=%u playMs=%u playRate=%.6g repeatUntilStageEnd=%s",
					Clip.iSourceStartMs, Clip.iPlayMs, Clip.fPlayRate,
					Clip.bLoop ? "true" : "false");
				ImGui::Unindent();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::EndChild();

	ImGui::TableSetColumnIndex(2);
	ImGui::BeginChild(
		"##ValtanWorkbenchDetail", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders);
	ImGui::SeparatorText("Persistent Detail");
	if (m_bValtanWorkbenchFocusDetailRequested)
	{
		ImGui::SetScrollHereY(0.f);
		m_bValtanWorkbenchFocusDetailRequested = false;
	}
	const char_t* pSelectionKind = "Stage";
	switch (m_eValtanWorkbenchSelection)
	{
	case VALTAN_WORKBENCH_SELECTION_KIND::TARGET:
		pSelectionKind = "Target";
		break;
	case VALTAN_WORKBENCH_SELECTION_KIND::PATTERN:
		pSelectionKind = "Pattern";
		break;
	case VALTAN_WORKBENCH_SELECTION_KIND::STAGE:
		pSelectionKind = "Stage";
		break;
	}
	ImGui::TextDisabled("Selection: %s", pSelectionKind);
	ImGui::TextWrapped(
		"target boss.valtan | pattern %s | stage %s",
		m_strValtanWorkbenchPatternId.c_str(),
		m_strValtanWorkbenchStageId.empty() ? "NONE" :
			m_strValtanWorkbenchStageId.c_str());
	const auto DetailOwnerButton = [this](
		const char_t* pLabel,
		const VALTAN_WORKBENCH_DETAIL_OWNER eOwner)
	{
		if (ImGui::RadioButton(
			pLabel, eOwner == m_eValtanWorkbenchDetailOwner))
		{
			m_eValtanWorkbenchDetailOwner = eOwner;
		}
	};
	DetailOwnerButton(
		"Gameplay", VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY);
	ImGui::SameLine();
	DetailOwnerButton(
		"Animation", VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION);
	ImGui::SameLine();
	DetailOwnerButton("Effect", VALTAN_WORKBENCH_DETAIL_OWNER::EFFECT);
	DetailOwnerButton("Sound", VALTAN_WORKBENCH_DETAIL_OWNER::SOUND);
	ImGui::SameLine();
	DetailOwnerButton("Camera", VALTAN_WORKBENCH_DETAIL_OWNER::CAMERA);
	ImGui::SameLine();
	DetailOwnerButton("World", VALTAN_WORKBENCH_DETAIL_OWNER::WORLD);

	if (VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY ==
		m_eValtanWorkbenchDetailOwner)
	{
		RenderPersistentSave();
		if (VALTAN_WORKBENCH_SELECTION_KIND::TARGET ==
			m_eValtanWorkbenchSelection)
		{
			ImGui::SeparatorText("Target Detail");
			ImGui::TextWrapped(
				"Valtan is the Product Server-authority target. Select a pattern or stage in the Outliner to edit its admitted typed gameplay draft.");
		}
		else if (nullptr != pSelectedStage)
		{
			ImGui::SeparatorText("Selected Stage Draft");
			ImGui::TextWrapped(
				"%s / %s | action %s | %u ms",
				pSelected->strPatternId.c_str(),
				pSelectedStage->strStageId.c_str(),
				pSelectedStage->strActionId.c_str(),
				pSelectedStage->iDurationMs);
			ImGui::BeginDisabled(!bMutationAdmitted);
			Render_ValtanCounterWindowInspector(
				*pSelected, *pSelectedStage);
			Render_ValtanStageDraftInspector(
				*pSelected, *pSelectedStage);
			ImGui::EndDisabled();
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::ANIMATION ==
		m_eValtanWorkbenchDetailOwner)
	{
		if (!bHasPreviewModel)
		{
			ImGui::SeparatorText("Animation Sequence Owner (read-only data mode)");
			ImGui::TextWrapped(
				"Saved Animation occurrence/sequence rows remain visible in the joined Sequencer below. Add, reorder and Save require the exact Valtan Model View clip vocabulary and native durations.");
		}
		else if (nullptr == pSelectedStage ||
			VALTAN_WORKBENCH_SELECTION_KIND::TARGET ==
				m_eValtanWorkbenchSelection)
		{
			ImGui::TextDisabled(
				"Select a pattern or semantic stage to open its typed animation sequence owner.");
		}
		else
		{
			if (Render_ValtanAnimationBindingInspector(
				pModel, *pSelected, *pSelectedStage))
			{
				bReloadPatternMasterAfterSave = true;
			}
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::EFFECT ==
		m_eValtanWorkbenchDetailOwner)
	{
		ImGui::SeparatorText("Effect Owner (read-only here)");
		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				pSelectedStage->ProductCues)
			{
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				ImGui::TextWrapped(
					"Data/Effects/Authored/%s.effect.json",
					Cue.strEffectAssetId.c_str());
				if (ImGui::SmallButton("Open Effect Tool##PersistentDetail"))
				{
					m_strEffectToolOpenPatternId = pSelected->strPatternId;
					m_strEffectToolOpenStageId = pSelectedStage->strStageId;
					m_strEffectToolOpenCueOccurrenceId = Cue.strOccurrenceId;
					m_strEffectToolOpenEffectAssetId = Cue.strEffectAssetId;
					m_hasEffectToolOpenRequest = true;
				}
				ImGui::PopID();
			}
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::SOUND ==
		m_eValtanWorkbenchDetailOwner)
	{
		RenderPersistentSave();
		ImGui::BulletText(
			"Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json");
		ImGui::TextWrapped(
			"Server combat-object impact event rows remain typed-editable in the selected Sequencer rows and commit with the Product Save above.");
		if (!bHasPreviewModel)
		{
			ImGui::TextWrapped(
				"Saved Sound rows remain visible in the joined Sequencer. Add/Edit/Save of clip-bound Pattern Sound requires the Model View so clip duration admission cannot be guessed.");
		}
		else if (nullptr != pSelectedStage)
		{
			ImGui::BeginDisabled(!bMutationAdmitted);
			if (Render_ValtanPatternSoundInspector(
				pModel, *pSelected, *pSelectedStage))
			{
				bReloadPatternMasterAfterSave = true;
			}
			ImGui::EndDisabled();
		}
		else
		{
			ImGui::TextDisabled(
				"Select a semantic stage to edit its exact Pattern Sound rows.");
		}
	}
	else if (VALTAN_WORKBENCH_DETAIL_OWNER::CAMERA ==
		m_eValtanWorkbenchDetailOwner)
	{
		ImGui::SeparatorText("Camera Owner (read-only here)");
		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
				pSelectedStage->CameraInvocations)
			{
				ImGui::PushID(Invocation.strCameraInvocationId.c_str());
				ImGui::TextWrapped(
					"%s -> %s", Invocation.strCameraInvocationId.c_str(),
					Invocation.strCameraCueId.c_str());
				if (ImGui::SmallButton("Open Camera Tool##PersistentDetail"))
				{
					m_strCameraToolOpenCueId = Invocation.strCameraCueId;
					m_hasCameraToolOpenRequest = true;
				}
				ImGui::PopID();
			}
		}
	}
	else
	{
		ImGui::SeparatorText("World Owner (read-only here)");
		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_STAGE_BRANCH_VIEW& Branch :
				pSelectedStage->Branches)
			{
				ImGui::BulletText(
					"%s -> %s", Branch.strOutcome.c_str(),
					Branch.strNextActionId.has_value() ?
						Branch.strNextActionId->c_str() : "TERMINAL");
			}
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
				pSelected->WorldEventTriggerRefs)
			{
				if (Event.strStageId == pSelectedStage->strStageId)
					ImGui::BulletText("world event %s", Event.strTriggerKind.c_str());
			}
		}
	}
	ImGui::EndChild();
	ImGui::EndTable();
	}

	if (ImGui::BeginChild(
		"##ValtanWorkbenchSequencer", ImVec2(0.f, 520.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Sequencer / Joined Tracks");
		if (nullptr != pSelectedStage)
		{
			ImGui::TextDisabled(
				"Selected stage filter: %s / %s | action %s | %u ms",
				pSelected->strPatternId.c_str(),
				pSelectedStage->strStageId.c_str(),
				pSelectedStage->strActionId.c_str(),
				pSelectedStage->iDurationMs);
			ImGui::BeginDisabled(!bMutationAdmitted);
			Render_ValtanPresentationLanes(
				*pSelected, pSelectedStage->strStageId);
			ImGui::EndDisabled();
		}
		else
		{
			ImGui::TextDisabled(
				"Select a semantic stage to join Animation, Effect, Sound, Camera, and World owner tracks.");
		}
	}
	ImGui::EndChild();

	if (ImGui::BeginChild(
		"##ValtanWorkbenchDataFiles", ImVec2(0.f, 360.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::SeparatorText("Data Files");
		ImGui::TextDisabled(
			"Exact owners for the current stable selection. JSON is never edited generically from this pane.");
		if (!m_strValtanPatternMasterStatus.empty())
		{
			ImGui::TextWrapped(
				"Canonical join: %s",
				m_strValtanPatternMasterStatus.c_str());
		}
		Render_ValtanSelectedResourceUsage(*pSelected, pSelectedStage);

		ImGui::BeginDisabled(nullptr == m_pBalanceTool);
		if (ImGui::SmallButton("Open Valtan Balance / Gameplay##DataFiles"))
			m_pBalanceTool->Open_Valtan();
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!bHasPreviewModel);
		if (ImGui::SmallButton("Open Animation Sequence Intake##DataFiles"))
			m_bShowValtanCustomChainWindow = true;
		ImGui::EndDisabled();

		if (nullptr != pSelectedStage)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				pSelectedStage->ProductCues)
			{
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				if (ImGui::SmallButton("Open Effect Tool"))
				{
					m_strEffectToolOpenPatternId =
						pSelected->strPatternId;
					m_strEffectToolOpenStageId =
						pSelectedStage->strStageId;
					m_strEffectToolOpenCueOccurrenceId =
						Cue.strOccurrenceId;
					m_strEffectToolOpenEffectAssetId =
						Cue.strEffectAssetId;
					m_hasEffectToolOpenRequest = true;
				}
				ImGui::SameLine();
				ImGui::TextDisabled(
					"%s", Cue.strEffectAssetId.c_str());
				ImGui::PopID();
			}
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Invocation :
				pSelectedStage->CameraInvocations)
			{
				ImGui::PushID(Invocation.strCameraInvocationId.c_str());
				if (ImGui::SmallButton("Open Camera Tool"))
				{
					m_strCameraToolOpenCueId = Invocation.strCameraCueId;
					m_hasCameraToolOpenRequest = true;
				}
				ImGui::SameLine();
				ImGui::TextDisabled(
					"%s", Invocation.strCameraCueId.c_str());
				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
	if (bReloadPatternMasterAfterSave)
	{
		/* The previous inline transaction reloaded before publishing its final
		   user-facing Save status. Keep that observable order while deferring the
		   pointer-invalidating reload until every pane has finished this frame. */
		std::string SaveStatus = m_strValtanPatternMasterStatus;
		if (Reload_ValtanPatternMaster())
		{
			m_strValtanPatternMasterStatus = std::move(SaveStatus);
		}
		else
		{
			const std::string ReloadDiagnostic =
				m_strValtanPatternMasterStatus;
			m_strValtanPatternMasterStatus = std::move(SaveStatus) +
				" Joined Workbench reload rejected; previous admitted view preserved. " +
				ReloadDiagnostic;
		}
	}
	}
}

void Client::CAnimation_Tool::Render_ValtanWorkspaceTabs(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!ImGui::BeginTabBar("##ValtanWorkspaceTabs"))
		return;

	const ImGuiTabItemFlags PatternFlags =
		m_bValtanWorkspaceTabInitialized ? ImGuiTabItemFlags_None :
			ImGuiTabItemFlags_SetSelected;
	m_bValtanWorkspaceTabInitialized = true;
	if (ImGui::BeginTabItem("Pattern Workbench", nullptr, PatternFlags))
	{
		Render_ValtanPatternMaster(pModel);
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Animation Clips / Sequence Intake"))
	{
		Render_ValtanAnimationSourceWorkspace(pModel);
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
}

void Client::CAnimation_Tool::Render_ValtanAnimationSourceWorkspace(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Animation Clips / Sequence Intake");
	if (nullptr == pModel)
	{
		ImGui::TextWrapped(
			"The exact Valtan Model View is required for local playback, source reference 1-67, Skill Timing, clip inventory and Animation Sequence Intake. The Pattern Workbench remains fully available in the first tab.");
		return;
	}

	/* Reference and clip documents are deliberately admitted only while this
	   source tab is visible.  The default Product tab never opens Skill Timing
	   or parses the large source/reference inventories as incidental UI. */
	if (!m_bClipMapLoadAttempted)
	{
		m_bClipMapLoadAttempted = true;
		Load_ClipMap();
	}
	if (!m_bClipSeqLoadAttempted)
	{
		m_bClipSeqLoadAttempted = true;
		Load_ClipSeq();
	}
	if (!m_bClipNotifyLoadAttempted)
	{
		m_bClipNotifyLoadAttempted = true;
		Load_ClipNotify();
	}
	if (!m_bRefLoadAttempted)
	{
		m_bRefLoadAttempted = true;
		Load_SkillReference();
	}

	ImGui::Text(
		"Asset: Valtan   Animations: %u", pModel->Get_NumAnimations());
	ImGui::TextDisabled(
		"REFERENCE / INTAKE: local playback never changes Server gameplay or Product owners until an explicit typed promotion succeeds.");
	ImGui::BeginDisabled(
		m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
	Render_Playback(pModel);
	ImGui::EndDisabled();
	Render_ValtanPatternPreview(pModel);

	ImGui::SeparatorText("Animation Sequence Intake");
	if (ImGui::SmallButton("Open Animation Sequence Intake"))
		m_bShowValtanCustomChainWindow = true;
	if (m_bShowValtanCustomChainWindow)
		Render_ValtanCustomChainWindow(pModel);

	ImGui::BeginDisabled(
		m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
	Render_NotifyReference(pModel);
	Render_HitAreaWires(pModel);
	Render_SkillReference(pModel, true);
	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::SeparatorText("Clips");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##filter", "filter by name", m_Filter, sizeof(m_Filter));
	Render_AnimationList(pModel);
	ImGui::EndDisabled();
}

void Client::CAnimation_Tool::Render_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Secondary / Read-only Source Reference (1-67)");
	ImGui::TextWrapped(
		"Historical reference only: Valtan.patternpreview.json and Valtan.clipseq are source evidence, not the Product pattern master. Every source clip plays once for review; environment, movement, effects, and damage are not simulated here.");

	if (!m_bValtanPatternPreviewLoadAttempted)
	{
		m_bValtanPatternPreviewLoadAttempted = true;
		std::string status;
		VALTAN_PATTERN_PREVIEW_DOCUMENT staged;
		if (CValtanPatternPreviewDocument::Load(
				"Valtan", Collect_ClipNames(pModel), staged, status))
		{
			m_ValtanPatternPreviewDocument = std::move(staged);
			m_strValtanPatternPreviewStatus = status;
		}
		else
		{
			m_strValtanPatternPreviewStatus =
				"Pattern preview unavailable; current animation preserved: " + status;
		}
	}

	const bool_t bReady =
		67u == m_ValtanPatternPreviewDocument.Patterns.size();
	ImGui::BeginDisabled(!bReady);
	if (ImGui::Button("Play All 1-67"))
		Start_ValtanPatternPreview(pModel, 1u, 67u);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
			"Speed##ValtanPatternPreview",
			&m_fValtanPatternPreviewSpeed,
			0.5f,
			3.f,
			"%.1fx"))
	{
		if (m_bValtanPatternPreviewPlaying)
			pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
	}

	const int32_t iPatternCount = static_cast<int32_t>(
		m_ValtanPatternPreviewDocument.Patterns.size());
	if (iPatternCount > 0)
	{
		m_iValtanPatternPreviewSelected = std::clamp(
			m_iValtanPatternPreviewSelected, 0, iPatternCount - 1);
	}
	if (ImGui::Button("< Pattern") && m_iValtanPatternPreviewSelected > 0)
	{
		--m_iValtanPatternPreviewSelected;
		Start_ValtanPatternPreview(
			pModel,
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1),
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1));
	}
	ImGui::SameLine();
	if (ImGui::Button("Local Pattern Preview") && iPatternCount > 0)
	{
		const uint32_t Number =
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1);
		Start_ValtanPatternPreview(pModel, Number, Number);
	}
	ImGui::SameLine();
	if (ImGui::Button("Pattern >") &&
		m_iValtanPatternPreviewSelected + 1 < iPatternCount)
	{
		++m_iValtanPatternPreviewSelected;
		Start_ValtanPatternPreview(
			pModel,
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1),
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1));
	}

	ImGui::BeginDisabled(!m_bValtanPatternPreviewPlaying);
	if (ImGui::Button(m_bValtanPatternPreviewPaused ? "Resume" : "Pause Sequence"))
	{
		m_bValtanPatternPreviewPaused = !m_bValtanPatternPreviewPaused;
		pModel->Set_AnimPaused(m_bValtanPatternPreviewPaused);
	}
	ImGui::SameLine();
	if (ImGui::Button("Replay Step"))
		Activate_ValtanPatternPreviewItem(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Skip Step"))
		Advance_ValtanPatternPreview(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		Stop_ValtanPatternPreview(pModel, "Pattern preview stopped; idle restored.");
	ImGui::EndDisabled();
	ImGui::EndDisabled();

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size() &&
		m_ValtanPatternPreviewPlaylist[
			m_iValtanPatternPreviewItem].iPatternNumber >= 1u)
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		m_iValtanPatternPreviewSelected = static_cast<int32_t>(
			Item.iPatternNumber - 1u);
	}

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		ImGui::Text(
			"Now %02u  [%s]  step %u/%u  %s",
			Item.iPatternNumber,
			CValtanPatternPreviewDocument::Evidence_Name(Item.eEvidence),
			Item.iStepNumber,
			Item.iStepCount,
			Item.bPatternMarker ? "(no body animation)" : Item.strClipName.c_str());
		if (Item.bPatternMarker)
		{
			ImGui::TextDisabled("Source sequence: pattern marker only");
		}
		else
		{
			ImGui::Text(
				"Source action %u  sequence %d  repeat %u/%u  source step %u/%u",
				Item.iSourceActionId,
				Item.iSequenceIndex,
				Item.iSequenceRepeatNumber,
				Item.iSequenceRepeatCount,
				Item.iSourceStepNumber,
				Item.iSourceStepCount);
			ImGui::TextWrapped(
				"%s  [%s]",
				Item.strSequenceName.c_str(),
				Item.strSequenceMode.c_str());
		}
		const f32_t Duration = (std::max)(
			0.001f, m_fValtanPatternPreviewItemDurationSeconds);
		ImGui::ProgressBar(
			std::clamp(m_fValtanPatternPreviewElapsedSeconds / Duration, 0.f, 1.f),
			ImVec2(-1.f, 0.f));
		ImGui::TextWrapped("%s", Item.strPatternLabel.c_str());
		ImGui::TextDisabled("%s", Item.strNote.c_str());
	}
	if (!m_strValtanPatternPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternPreviewStatus.c_str());

	if (bReady && iPatternCount > 0)
	{
		const VALTAN_PATTERN_PREVIEW_ENTRY& SelectedPattern =
			m_ValtanPatternPreviewDocument.Patterns[
				static_cast<std::size_t>(m_iValtanPatternPreviewSelected)];
		ImGui::SeparatorText("Selected source sequences");
		if (SelectedPattern.Sequences.empty())
		{
			ImGui::TextDisabled("No body-animation source sequence for this pattern.");
		}
		else
		{
			for (const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Sequence :
				SelectedPattern.Sequences)
			{
				ImGui::BulletText(
					"action %u  sequence %d  repeat x%u  [%s]",
					Sequence.iSourceActionId,
					Sequence.iSequenceIndex,
					Sequence.iRepeat,
					Sequence.strSequenceMode.c_str());
				ImGui::Indent();
				ImGui::TextWrapped("%s", Sequence.strSequenceName.c_str());
				ImGui::Unindent();
			}
		}
	}

	if (ImGui::SmallButton("Open Read-only Source Sequence Window"))
		m_bShowValtanSourceReferenceWindow = true;
	if (m_bShowValtanSourceReferenceWindow)
		Render_ValtanPatternReferenceWindow(pModel);

	if (bReady)
	{
		const bool_t bListVisible = ImGui::BeginChild(
			"##ValtanPatternPreviewList",
			ImVec2(0.f, 260.f),
			ImGuiChildFlags_Borders,
			ImGuiWindowFlags_NoScrollWithMouse);
		if (bListVisible)
		{
			for (int32_t Index = 0; Index < iPatternCount; ++Index)
			{
				const VALTAN_PATTERN_PREVIEW_ENTRY& Pattern =
					m_ValtanPatternPreviewDocument.Patterns[Index];
				char_t Label[512]{};
				snprintf(
					Label,
					sizeof(Label),
					"%02u  [%s]  %s",
					Pattern.iNumber,
					CValtanPatternPreviewDocument::Evidence_Name(Pattern.eEvidence),
					Pattern.strLabel.c_str());
				ImGui::PushID(Index);
				if (ImGui::Selectable(
						Label, Index == m_iValtanPatternPreviewSelected))
				{
					m_iValtanPatternPreviewSelected = Index;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", Pattern.strNote.c_str());
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
}

void Client::CAnimation_Tool::Render_ValtanPatternReferenceWindow(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SetNextWindowSize(ImVec2(460.f, 560.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(540.f, 60.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(
		"Valtan Source Reference (Read-only)",
		&m_bShowValtanSourceReferenceWindow))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"Secondary source evidence only. One button per read-only Valtan.clipseq sequence; Product authoring must use Valtan Pattern Master in the main Animation Tool window.");

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		ImGui::Text("Now step %u/%u  %s",
			Item.iStepNumber,
			Item.iStepCount,
			Item.bPatternMarker ? "(no body animation)" :
				Item.strClipName.c_str());
		ImGui::TextWrapped("%s", Item.strPatternLabel.c_str());
		const f32_t Duration = (std::max)(
			0.001f, m_fValtanPatternPreviewItemDurationSeconds);
		ImGui::ProgressBar(
			std::clamp(
				m_fValtanPatternPreviewElapsedSeconds / Duration, 0.f, 1.f),
			ImVec2(-1.f, 0.f));
		if (ImGui::Button(
			m_bValtanPatternPreviewPaused ? "Resume" : "Pause"))
		{
			m_bValtanPatternPreviewPaused = !m_bValtanPatternPreviewPaused;
			pModel->Set_AnimPaused(m_bValtanPatternPreviewPaused);
		}
		ImGui::SameLine();
		if (ImGui::Button("Replay"))
			Activate_ValtanPatternPreviewItem(pModel);
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			Stop_ValtanPatternPreview(
				pModel, "Pattern preview stopped; idle restored.");
		}
	}
	else
	{
		ImGui::TextDisabled("Nothing playing.");
	}
	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
			"Speed##ValtanPatternReference",
			&m_fValtanPatternPreviewSpeed, 0.5f, 3.f, "%.1fx") &&
		m_bValtanPatternPreviewPlaying)
	{
		pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
	}

	ImGui::Checkbox(
		"Raid patterns only (420xxx)", &m_bValtanRaidSequencesOnly);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##valtanpatternfilter",
		"filter by skill id or name",
		m_ValtanPatternFilter,
		sizeof(m_ValtanPatternFilter));

	if (ImGui::BeginChild(
		"##ValtanPatternReferenceButtons",
		ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (m_ClipSeqs.empty())
		{
			ImGui::TextDisabled(
				"No clip sequences loaded from Valtan.clipseq.");
		}
		const int32_t iSequenceCount =
			static_cast<int32_t>(m_ClipSeqs.size());
		for (int32_t Index = 0; Index < iSequenceCount; ++Index)
		{
			const CLIP_SEQ& Seq = m_ClipSeqs[Index];
			if (m_bValtanRaidSequencesOnly &&
				(Seq.iSkillId < 420000 || Seq.iSkillId > 429999))
			{
				continue;
			}
			char_t SkillId[16]{};
			snprintf(SkillId, sizeof(SkillId), "%d", Seq.iSkillId);
			if (!Contains_NoCase(SkillId, m_ValtanPatternFilter) &&
				!Contains_NoCase(Seq.name.c_str(), m_ValtanPatternFilter))
			{
				continue;
			}
			char_t Label[512]{};
			snprintf(Label, sizeof(Label), "%s  seq%d [%s]  (%zu clips)##%d",
				Seq.name.empty() ? SkillId : Seq.name.c_str(),
				Seq.iSeqIndex,
				Seq.sMode.c_str(),
				Seq.clips.size(),
				Index);
			ImGui::PushID(Index);
			const bool_t bPlayingThis = m_bValtanPatternPreviewPlaying &&
				m_iValtanSequenceSelected == Index;
			if (bPlayingThis)
			{
				ImGui::PushStyleColor(
					ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.24f, 1.f));
			}
			if (ImGui::Button(Label, ImVec2(-1.f, 0.f)))
				Start_ValtanSequencePreview(
					pModel, static_cast<std::size_t>(Index));
			if (bPlayingThis)
				ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				std::string clipList;
				for (const std::string& clip : Seq.clips)
				{
					if (!clipList.empty())
						clipList += "\n";
					clipList += clip;
				}
				ImGui::SetTooltip("skill %d  seq %d\n%s",
					Seq.iSkillId, Seq.iSeqIndex, clipList.c_str());
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::End();
}
