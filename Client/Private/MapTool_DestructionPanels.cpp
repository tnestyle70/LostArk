#include "imgui.h"
#include "MapTool_Internal.h"
#include "DeployPropObject.h"
#include "DestructionSimulationController.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include "Model.h"




void Client::CMapTool::Render_DestructionSimpleEditor()
{
	if (ImGui::Button(m_bDestructionPickArmed ?
		"Cancel Wall Picking" : "Pick Wall In Viewport"))
	{
		m_bDestructionPickArmed = !m_bDestructionPickArmed;
		if (!m_bDestructionPickArmed)
			m_bDestructionAddMemberArmed = false;
		m_DestructionStatus = m_bDestructionPickArmed ?
			"Wall picking armed. Click a wall in the uncovered game viewport." :
			"Wall picking cancelled";
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Authoring Data"))
	{
		Reload_DestructionAuthoring();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_DestructionDocument.Is_Dirty());
	if (ImGui::Button("Save Pending Changes"))
		Save_WorldDestruction();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_DestructionDocument.Is_Dirty() ? "[unsaved]" : "[saved]");

	if (m_bDestructionPickArmed)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PICKING: click the world view, not this ImGui window. Escape cancels.");
	}
	ImGui::TextWrapped("%s", m_DestructionStatus.c_str());
	ImGui::Separator();

	if (ImGui::BeginTable("SimpleDestructionLayout", 2,
		ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("Walls", ImGuiTableColumnFlags_WidthFixed, 360.f);
		ImGui::TableSetupColumn("Selected Wall", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_DestructionSimpleWallList();
		ImGui::TableSetColumnIndex(1);
		Render_DestructionSimpleInspector();
		ImGui::EndTable();
	}
}

void Client::CMapTool::Render_DestructionSimpleWallList()
{
	ImGui::TextUnformatted("1. Choose a wall");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##SimpleWallFilter", "Search asset or placement ID",
		m_DestructionDeployFilter, IM_ARRAYSIZE(m_DestructionDeployFilter));
	ImGui::Checkbox("Only walls without a break setting",
		&m_bDestructionOnlyUnassigned);

	const std::vector<DEPLOY_RUNTIME_ENTRY>& entries =
		m_DeployRuntime.Get_Entries();
	const size_t destructibleCount = static_cast<size_t>(std::count_if(
		entries.begin(), entries.end(), [](const DEPLOY_RUNTIME_ENTRY& entry)
		{
			return entry.placement.destructible;
		}));
	ImGui::Text("%zu destructible walls loaded", destructibleCount);
	const f32_t height = (std::max)(360.f, ImGui::GetContentRegionAvail().y);
	if (!ImGui::BeginChild("SimpleWallRows", ImVec2(0.f, height), true))
	{
		ImGui::EndChild();
		return;
	}

	const std::string filter = m_DestructionDeployFilter;
	for (const DEPLOY_RUNTIME_ENTRY& entry : entries)
	{
		if (!entry.placement.destructible)
			continue;
		const DESTRUCTION_GROUP* owner =
			m_DestructionDocument.Find_GroupOfMember(
				entry.placement.runtimePlacementId);
		if (m_bDestructionOnlyUnassigned && nullptr != owner)
			continue;
		const std::string idText =
			std::to_string(entry.placement.runtimePlacementId);
		if (!filter.empty() &&
			!MatchesFilter(entry.placement.assetId, filter.c_str()) &&
			!MatchesFilter(idText, filter.c_str()))
		{
			continue;
		}

		const std::string label = std::string(nullptr != owner ? "[SET] " :
			"[NEW] ") + entry.placement.assetId + " / " + idText;
		const bool_t selected = m_iSelectedDeployPlacementId ==
			entry.placement.runtimePlacementId;
		ImGui::PushID(idText.c_str());
		if (ImGui::Selectable(label.c_str(), selected))
			Select_DestructionWall(entry.placement.runtimePlacementId, "wall list");
		if (nullptr != owner && ImGui::IsItemHovered())
			ImGui::SetTooltip("Group: %s", owner->groupId.c_str());
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_DestructionSimpleInspector()
{
	ImGui::TextUnformatted("2. Set when this wall breaks");
	const auto entry = std::find_if(
		m_DeployRuntime.Get_Entries().begin(),
		m_DeployRuntime.Get_Entries().end(),
		[this](const DEPLOY_RUNTIME_ENTRY& value)
		{
			return value.placement.runtimePlacementId ==
				m_iSelectedDeployPlacementId;
		});
	if (m_DeployRuntime.Get_Entries().end() == entry)
	{
		ImGui::TextWrapped(
			"Select a wall from the list or press Pick Wall In Viewport.");
		return;
	}

	const DEPLOY_PROP_ASSET_ENTRY* asset =
		m_DeployRuntime.Get_Catalog().Find(entry->placement.assetId);
	const DESTRUCTION_GROUP* owner =
		m_DestructionDocument.Find_GroupOfMember(
			entry->placement.runtimePlacementId);
	ImGui::SeparatorText("Selected Wall");
	ImGui::Text("Asset: %s", entry->placement.assetId.c_str());
	ImGui::Text("Placement: %llu",
		static_cast<unsigned long long>(entry->placement.runtimePlacementId));
	ImGui::Text("Position: %.2f, %.2f, %.2f",
		entry->placement.position.x, entry->placement.position.y,
		entry->placement.position.z);
	ImGui::Text("Source off action: %u | trigger evidence: %u",
		entry->placement.stateOffActionId,
		entry->placement.triggerBinaryOccurrenceCount);
	if (nullptr != asset)
	{
		ImGui::Text("Model: %s | fractured mesh: %s",
			DEPLOY_PROP_MODEL_KIND::ANIM == asset->kind ? "ANIM" : "STATIC",
			asset->fracturedRelativePath.empty() ? "NO" : "YES");
	}
	ImGui::Text("Break group: %s", nullptr != owner ?
		owner->groupId.c_str() : "not assigned (created automatically on Save)");
	if (nullptr != owner)
		ImGui::Text("Walls that break together: %zu",
			owner->memberPlacementIds.size());

	const char_t* previewScope = nullptr != owner ? "Group" : "Wall";
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" Original").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::INTACT);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" Broken").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::FRACTURED);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" Hidden").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::DESPAWNED);

	if (nullptr != owner)
	{
		if (!m_bDestructionAddMemberArmed)
		{
			if (ImGui::Button("Add Another Wall To This Group"))
			{
				m_bDestructionAddMemberArmed = true;
				m_bDestructionPickArmed = true;
				m_DestructionStatus =
					"Choose the next wall in the viewport or wall list";
			}
		}
		else
		{
			ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
				"Choose one more wall for group %s", owner->groupId.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Cancel Add"))
			{
				m_bDestructionAddMemberArmed = false;
				m_bDestructionPickArmed = false;
			}
		}
	}

	std::vector<const DESTRUCTION_BINDING*> groupBindings;
	if (nullptr != owner)
	{
		for (const DESTRUCTION_BINDING& binding :
			m_DestructionDocument.Get_Bindings())
		{
			const DESTRUCTION_MUTATION* mutation =
				m_DestructionDocument.Find_Mutation(binding.mutationId);
			if (nullptr != mutation && mutation->groupId == owner->groupId &&
				WORLD_DESTROYABLE_STATE::FRACTURED == mutation->eTargetState)
				groupBindings.push_back(&binding);
		}
	}
	if (!groupBindings.empty())
	{
		if (groupBindings.size() > 1u &&
			m_SelectedDestructionBindingId.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
				"This group has %zu break settings. Choose one explicitly.",
				groupBindings.size());
		}
		const char_t* preview = m_SelectedDestructionBindingId.empty() ?
			"Choose an existing setting" :
			m_SelectedDestructionBindingId.c_str();
		if (ImGui::BeginCombo("Existing Break Setting", preview))
		{
			for (const DESTRUCTION_BINDING* binding : groupBindings)
			{
				const bool_t selected = binding->bindingId ==
					m_SelectedDestructionBindingId;
				const std::string label = binding->patternId + " / " +
					binding->stageId + "##" + binding->bindingId;
				if (ImGui::Selectable(label.c_str(), selected))
					Load_DestructionDraftFromBinding(*binding);
			}
			ImGui::EndCombo();
		}
		ImGui::SameLine();
		if (ImGui::Button("New Setting"))
		{
			m_SelectedDestructionBindingId.clear();
			m_SelectedDestructionPatternId.clear();
			m_SelectedDestructionStageId.clear();
			m_iDestructionTriggerKind = 1;
			m_iDestructionOffsetMs = 0;
			m_iDestructionBreakingMs = 1900;
			m_bDestructionBindingEnabled = false;
			m_DestructionReceiverId[0] = '\0';
			m_fDestructionTimelineMs = 0.f;
			m_bDestructionTimelinePlaying = false;
			m_bDestructionNewSettingArmed = true;
		}
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	const char_t* patternPreview = nullptr != pattern ?
		pattern->displayName.c_str() : "Choose a Valtan pattern";
	if (ImGui::BeginCombo("Valtan Pattern", patternPreview))
	{
		for (const ENCOUNTER_PATTERN_REFERENCE& candidate :
			m_EncounterReference.Get_Patterns())
		{
			const bool_t selected = candidate.patternId ==
				m_SelectedDestructionPatternId;
			const std::string label = candidate.displayName + "##" +
				candidate.patternId;
			if (ImGui::Selectable(label.c_str(), selected))
			{
				m_SelectedDestructionPatternId = candidate.patternId;
				m_SelectedDestructionStageId = candidate.stages.empty() ? "" :
					candidate.stages.front().stageId;
				m_iDestructionOffsetMs = 0;
				m_fDestructionTimelineMs = 0.f;
				m_bDestructionTimelinePlaying = false;
			}
		}
		ImGui::EndCombo();
	}

	pattern = m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	const ENCOUNTER_STAGE_REFERENCE* stage = Find_SelectedDestructionStage();
	const char_t* stagePreview = nullptr != stage ?
		stage->stageId.c_str() : "Choose a stage";
	ImGui::BeginDisabled(nullptr == pattern);
	if (ImGui::BeginCombo("Animation Stage", stagePreview))
	{
		for (const ENCOUNTER_STAGE_REFERENCE& candidate : pattern->stages)
		{
			const bool_t selected = candidate.stageId ==
				m_SelectedDestructionStageId;
			const std::string label = candidate.stageId + " / " +
				candidate.stageKind + " / " + candidate.actionId;
			if (ImGui::Selectable(label.c_str(), selected))
			{
				m_SelectedDestructionStageId = candidate.stageId;
				m_iDestructionOffsetMs = 0;
				m_fDestructionTimelineMs =
					static_cast<f32_t>(candidate.iStartOffsetMs);
			}
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	stage = Find_SelectedDestructionStage();
	if (nullptr != stage)
	{
		ImGui::Text("Stage actionId: %s | %s | %u ms",
			stage->actionId.c_str(), stage->stageKind.c_str(),
			stage->iDurationMs);
		const char_t* genericClip = "patternRecovery";
		if ("WINDUP" == stage->stageKind)
			genericClip = "patternWindup";
		else if ("ACTIVE" == stage->stageKind)
			genericClip = "patternActive";
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"Runtime currently resolves the generic %s clip; exact actionId clip is not authored.",
			genericClip);
	}

	Render_DestructionSimpleTimeline();
	ImGui::SeparatorText("Break Condition");
	if (ImGui::RadioButton("At stage start", 0 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("At selected time", 1 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 1;
	ImGui::SameLine();
	if (ImGui::RadioButton("At stage end", 2 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 2;
	ImGui::SameLine();
	if (ImGui::RadioButton("On boss collision", 3 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 3;

	if (1 == m_iDestructionTriggerKind && nullptr != stage)
	{
		m_iDestructionOffsetMs = (std::clamp)(m_iDestructionOffsetMs, 0,
			static_cast<int32_t>(stage->iDurationMs));
		if (ImGui::SliderInt("Break point inside stage (ms)",
			&m_iDestructionOffsetMs, 0,
			static_cast<int32_t>(stage->iDurationMs)))
		{
			m_fDestructionTimelineMs = static_cast<f32_t>(
				stage->iStartOffsetMs + m_iDestructionOffsetMs);
		}
	}
	if (3 == m_iDestructionTriggerKind)
	{
		const char_t* receiverPreview = '\0' != m_DestructionReceiverId[0] ?
			m_DestructionReceiverId : "Choose a Collision Box";
		if (ImGui::BeginCombo("Boss Collision Receiver", receiverPreview))
		{
			for (const WORLD_GAMEPLAY_PLACEMENT& placement :
				m_WorldGameplayDocument.Get_Placements())
			{
				if (WORLD_PLACEMENT_KIND::COLLISION_BOX != placement.eKind)
					continue;
				if (ImGui::Selectable(placement.placementId.c_str(),
					placement.placementId == m_DestructionReceiverId))
				{
					strncpy_s(m_DestructionReceiverId,
						placement.placementId.c_str(), _TRUNCATE);
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::InputInt("Group breaking presentation duration (ms)",
		&m_iDestructionBreakingMs, 50, 250);
	m_iDestructionBreakingMs = (std::clamp)(m_iDestructionBreakingMs, 0,
		static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS));
	ImGui::Checkbox("Enable this break setting", &m_bDestructionBindingEnabled);
	if (!m_bDestructionBindingEnabled)
		ImGui::TextDisabled("Disabled is the safe authoring default until the Server path is published.");
	const bool_t navigationMissing = nullptr == owner ||
		owner->navigationRegionIds.empty();
	if (navigationMissing)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"Navigation blocker region: not linked (use Advanced Graph Editor after authoring one)." );
		if (m_bDestructionBindingEnabled)
			ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"Enabled settings cannot be saved until a navigation region is linked.");
	}

	const bool_t settingChoiceRequired = groupBindings.size() > 1u &&
		m_SelectedDestructionBindingId.empty() &&
		!m_bDestructionNewSettingArmed;
	if (settingChoiceRequired)
		ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
			"Choose an existing setting or press New Setting before applying.");
	const bool_t canSave = nullptr != pattern && nullptr != stage &&
		!settingChoiceRequired &&
		!(m_bDestructionBindingEnabled && navigationMissing);
	ImGui::BeginDisabled(!canSave);
	if (ImGui::Button("Apply And Save This Wall Setting"))
	{
		if (Apply_SimpleDestructionAuthoring())
			Save_WorldDestruction();
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"This writes Data authoring. Product Server destruction is still gated.");
}

void Client::CMapTool::Render_DestructionSimpleTimeline()
{
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern || 0u == pattern->iTotalDurationMs)
		return;

	ImGui::SeparatorText("Pattern Timing Preview");
	if (ImGui::Button(m_bDestructionTimelinePlaying ? "Pause" : "Play"))
		m_bDestructionTimelinePlaying = !m_bDestructionTimelinePlaying;
	ImGui::SameLine();
	if (ImGui::Button("Restart"))
	{
		m_fDestructionTimelineMs = 0.f;
		m_bDestructionTimelinePlaying = false;
	}
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_bDestructionTimelineLoop);

	const f32_t duration = static_cast<f32_t>(pattern->iTotalDurationMs);
	m_fDestructionTimelineMs = (std::clamp)(
		m_fDestructionTimelineMs, 0.f, duration);
	ImGui::SliderFloat("Pattern time", &m_fDestructionTimelineMs,
		0.f, duration, "%.0f ms");
	ImGui::SameLine();
	if (ImGui::Button("Use Current Time"))
		Use_DestructionTimelineTime();

	const f32_t width = (std::max)(220.f, ImGui::GetContentRegionAvail().x);
	constexpr f32_t barHeight = 28.f;
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
	{
		const f32_t left = origin.x + width *
			static_cast<f32_t>(stage.iStartOffsetMs) / duration;
		const f32_t right = origin.x + width *
			static_cast<f32_t>(stage.iStartOffsetMs + stage.iDurationMs) /
			duration;
		const bool_t selected = stage.stageId == m_SelectedDestructionStageId;
		const ImU32 fill = selected ? IM_COL32(65, 125, 205, 230) :
			("ACTIVE" == stage.stageKind ? IM_COL32(170, 85, 55, 210) :
				IM_COL32(65, 75, 100, 210));
		draw->AddRectFilled(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight), fill);
		draw->AddRect(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight),
			IM_COL32(220, 220, 220, 160));
		if (right - left > 48.f)
			draw->AddText(ImVec2(left + 4.f, origin.y + 6.f),
				IM_COL32(245, 245, 245, 255), stage.stageId.c_str());
	}
	const f32_t playheadX = origin.x + width *
		m_fDestructionTimelineMs / duration;
	draw->AddLine(ImVec2(playheadX, origin.y - 5.f),
		ImVec2(playheadX, origin.y + barHeight + 5.f),
		IM_COL32(255, 230, 70, 255), 2.f);
	ImGui::Dummy(ImVec2(width, barHeight + 10.f));
	ImGui::TextDisabled(
		"Timing preview only. It does not claim an exact Valtan animation clip.");
}

void Client::CMapTool::Render_DestructionGroupEditor()
{
	if (!ImGui::CollapsingHeader("Destruction Groups",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	ImGui::BeginDisabled(!m_DestructionDocument.Is_Dirty());
	if (ImGui::Button("Save World Events"))
		Save_AllAuthoring();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Reload World Events"))
		Load_WorldDestruction();
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_DestructionDocument.Is_Dirty() ? "[unsaved]" : "[saved]");
	ImGui::TextWrapped("%s", m_DestructionStatus.c_str());
	ImGui::Separator();

	ImGui::SetNextItemWidth(320.f);
	ImGui::InputText("New group ID", m_DestructionGroupId,
		IM_ARRAYSIZE(m_DestructionGroupId));
	ImGui::SameLine();
	if (ImGui::Button("Create Group"))
	{
		std::string status;
		if (m_DestructionDocument.Add_Group(m_DestructionGroupId, status))
			m_SelectedDestructionGroupId = m_DestructionGroupId;
		m_DestructionStatus = status;
		Refresh_DestructionHighlight();
	}

	if (ImGui::BeginTable("DestructionGroupList", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 140.f)))
	{
		ImGui::TableSetupColumn("groupId");
		ImGui::TableSetupColumn("members");
		ImGui::TableSetupColumn("navPolarity");
		ImGui::TableSetupColumn("initial");
		ImGui::TableHeadersRow();
		for (const DESTRUCTION_GROUP& entry : m_DestructionDocument.Get_Groups())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t isSelected =
				m_SelectedDestructionGroupId == entry.groupId;
			if (ImGui::Selectable(entry.groupId.c_str(), isSelected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedDestructionGroupId = entry.groupId;
				const auto profile = std::find_if(
					m_DestructionSimulationDocument.Get_Profiles().begin(),
					m_DestructionSimulationDocument.Get_Profiles().end(),
					[&entry](const DESTRUCTION_SIMULATION_PROFILE& value)
					{
						return value.groupId == entry.groupId;
					});
				if (profile !=
					m_DestructionSimulationDocument.Get_Profiles().end())
				{
					Select_DestructionSimulationProfile(profile->profileId);
				}
				Refresh_DestructionHighlight();
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%zu", entry.memberPlacementIds.size());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(
				CWorldDestructionDocument::NavPolarity_ToString(
					entry.eNavPolarity));
			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::DestroyableState_ToString(
					entry.eInitialState));
		}
		ImGui::EndTable();
	}

	/* Picking starts from one wall. Preview affects its whole selected group
	   when a group exists, or the single wall before its first group exists. */
	ImGui::Separator();
	ImGui::Checkbox("Pick walls in the world", &m_bDestructionPickArmed);
	ImGui::SameLine();
	ImGui::TextDisabled("Left click a wall in the viewport; Escape cancels");
	ImGui::Text("picked: %llu",
		static_cast<unsigned long long>(m_iSelectedDeployPlacementId));
	ImGui::SameLine();
	ImGui::TextDisabled("| preview is presentation only, never saved");
	const char_t* previewScope = m_SelectedDestructionGroupId.empty() ?
		"Wall" : "Group";
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" INTACT").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::INTACT);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" FRACTURED").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::FRACTURED);
	ImGui::SameLine();
	if (ImGui::Button((std::string("Preview ") + previewScope +
		" DESPAWNED").c_str()))
		Apply_DestructionPreview(DEPLOY_PROP_STATE::DESPAWNED);

	const DESTRUCTION_GROUP* group = m_SelectedDestructionGroupId.empty() ?
		nullptr :
		m_DestructionDocument.Find_Group(m_SelectedDestructionGroupId);
	if (nullptr == group)
	{
		ImGui::TextUnformatted(
			"Create a group above, then select it to add the picked wall.");
		return;
	}

	ImGui::Separator();
	ImGui::Text("Editing %s", group->groupId.c_str());
	if (ImGui::Button("Delete Group"))
	{
		CWorldDestructionDocument stagedDestruction = m_DestructionDocument;
		CDestructionSimulationDocument stagedSimulation =
			m_DestructionSimulationDocument;
		std::string status;
		const std::string removedGroupId = m_SelectedDestructionGroupId;
		if (stagedDestruction.Remove_Group(removedGroupId, status) &&
			stagedSimulation.Remove_ProfilesForGroup(removedGroupId, status) &&
			stagedSimulation.Validate_GroupReferences(
				stagedDestruction, status))
		{
			if (nullptr != m_pDestructionSimulationController)
				m_pDestructionSimulationController->Clear();
			m_DestructionDocument = std::move(stagedDestruction);
			m_DestructionSimulationDocument = std::move(stagedSimulation);
			Reset_DestructionSimulationUI();
			m_bDestructionSimulationClearRequested = false;
			m_SelectedDestructionGroupId.clear();
		}
		m_DestructionStatus = status;
		Refresh_DestructionHighlight();
		return;
	}

	/* The baked grid leaves every wall footprint walkable, so a wall blocks
	   while INTACT and a collapsing floor blocks while FRACTURED. */
	int32_t polarity = DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED ==
		group->eNavPolarity ? 1 : 0;
	ImGui::TextUnformatted("Navigation polarity");
	if (ImGui::RadioButton("Block while INTACT (wall)", 0 == polarity))
		polarity = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("Block while FRACTURED (floor)", 1 == polarity))
		polarity = 1;
	m_DestructionDocument.Set_NavPolarity(group->groupId,
		1 == polarity ? DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED :
		DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_INTACT);

	int32_t initialState =
		WORLD_DESTROYABLE_STATE::FRACTURED == group->eInitialState ? 1 : 0;
	ImGui::TextUnformatted("Initial state");
	if (ImGui::RadioButton("INTACT", 0 == initialState))
		initialState = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("FRACTURED", 1 == initialState))
		initialState = 1;
	m_DestructionDocument.Set_InitialState(group->groupId,
		1 == initialState ? WORLD_DESTROYABLE_STATE::FRACTURED :
		WORLD_DESTROYABLE_STATE::INTACT);

	ImGui::Separator();
	ImGui::BeginDisabled(0u == m_iSelectedDeployPlacementId);
	if (ImGui::Button("Add Picked Wall"))
	{
		if (Modify_DestructionGroupMember(
			m_iSelectedDeployPlacementId, true))
		{
			return;
		}
	}
	ImGui::EndDisabled();

	if (ImGui::BeginTable("DestructionMembers", 3,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 140.f)))
	{
		ImGui::TableSetupColumn("runtimePlacementId");
		ImGui::TableSetupColumn("assetId");
		ImGui::TableSetupColumn("");
		ImGui::TableHeadersRow();
		uint64_t removeRequest = 0u;
		for (const uint64_t placementId : group->memberPlacementIds)
		{
			ImGui::TableNextRow();
			ImGui::PushID(static_cast<int32_t>(placementId));
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%llu",
				static_cast<unsigned long long>(placementId));
			ImGui::TableSetColumnIndex(1);
			const shared_ptr<CDeployPropObject> prop =
				m_DeployRuntime.Find(placementId);
			ImGui::TextUnformatted(nullptr != prop ? "loaded" : "missing");
			ImGui::TableSetColumnIndex(2);
			if (ImGui::SmallButton("Remove"))
				removeRequest = placementId;
			ImGui::PopID();
		}
		ImGui::EndTable();
		if (0u != removeRequest &&
			Modify_DestructionGroupMember(removeRequest, false))
		{
			return;
		}
	}

	ImGui::SetNextItemWidth(280.f);
	ImGui::InputText("Nav region ID", m_DestructionRegionId,
		IM_ARRAYSIZE(m_DestructionRegionId));
	ImGui::SameLine();
	if (ImGui::Button("Link Region"))
	{
		std::string status;
		m_DestructionDocument.Add_NavigationRegion(
			group->groupId, m_DestructionRegionId, status);
		m_DestructionStatus = status;
	}
	std::string regionRemoveRequest;
	for (const std::string& regionId : group->navigationRegionIds)
	{
		ImGui::BulletText("%s", regionId.c_str());
		ImGui::SameLine();
		ImGui::PushID(regionId.c_str());
		if (ImGui::SmallButton("Unlink"))
			regionRemoveRequest = regionId;
		ImGui::PopID();
	}
	if (!regionRemoveRequest.empty())
	{
		m_DestructionDocument.Remove_NavigationRegion(
			m_SelectedDestructionGroupId, regionRemoveRequest);
	}

}

void Client::CMapTool::Render_DestructionBindingEditor()
{
	if (!ImGui::CollapsingHeader("Mutations and Bindings"))
		return;

	ImGui::SetNextItemWidth(300.f);
	ImGui::InputText("Mutation ID", m_DestructionMutationId,
		IM_ARRAYSIZE(m_DestructionMutationId));
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputInt("Breaking ms", &m_iDestructionBreakingMs, 50, 500);
	m_iDestructionBreakingMs = (std::clamp)(m_iDestructionBreakingMs, 0,
		static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS));
	ImGui::SameLine();
	ImGui::BeginDisabled(m_SelectedDestructionGroupId.empty());
	if (ImGui::Button("Create Mutation"))
	{
		DESTRUCTION_MUTATION mutation;
		mutation.mutationId = m_DestructionMutationId;
		mutation.groupId = m_SelectedDestructionGroupId;
		mutation.eTargetState = WORLD_DESTROYABLE_STATE::FRACTURED;
		mutation.iBreakingDurationMs =
			static_cast<uint32_t>(m_iDestructionBreakingMs);
		std::string status;
		m_DestructionDocument.Add_Mutation(mutation, status);
		m_DestructionStatus = status;
	}
	ImGui::EndDisabled();

	if (ImGui::BeginTable("DestructionMutations", 5,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("mutationId");
		ImGui::TableSetupColumn("groupId");
		ImGui::TableSetupColumn("target");
		ImGui::TableSetupColumn("breaking ms");
		ImGui::TableSetupColumn("");
		ImGui::TableHeadersRow();
		std::string removeRequest;
		for (const DESTRUCTION_MUTATION& entry :
			m_DestructionDocument.Get_Mutations())
		{
			ImGui::TableNextRow();
			ImGui::PushID(entry.mutationId.c_str());
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(entry.mutationId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(entry.groupId.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::DestroyableState_ToString(
					entry.eTargetState));
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", entry.iBreakingDurationMs);
			ImGui::TableSetColumnIndex(4);
			if (ImGui::SmallButton("Remove"))
				removeRequest = entry.mutationId;
			ImGui::PopID();
		}
		ImGui::EndTable();
		if (!removeRequest.empty())
		{
			std::string status;
			m_DestructionDocument.Remove_Mutation(removeRequest, status);
			m_DestructionStatus = status;
		}
	}

	ImGui::Separator();
	ImGui::SetNextItemWidth(300.f);
	ImGui::InputText("Binding ID", m_DestructionBindingId,
		IM_ARRAYSIZE(m_DestructionBindingId));
	ImGui::TextUnformatted("Trigger kind");
	if (ImGui::RadioButton("STAGE_ENTER", 0 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 0;
	ImGui::SameLine();
	if (ImGui::RadioButton("STAGE_TIME", 1 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 1;
	ImGui::SameLine();
	if (ImGui::RadioButton("STAGE_EXIT", 2 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 2;
	ImGui::SameLine();
	if (ImGui::RadioButton("COLLISION_IMPACT", 3 == m_iDestructionTriggerKind))
		m_iDestructionTriggerKind = 3;

	ImGui::BeginDisabled(1 != m_iDestructionTriggerKind);
	ImGui::SetNextItemWidth(200.f);
	ImGui::InputInt("Offset ms", &m_iDestructionOffsetMs, 10, 100);
	m_iDestructionOffsetMs = (std::clamp)(m_iDestructionOffsetMs, 0,
		static_cast<int32_t>(CWorldDestructionDocument::MAX_DURATION_MS));
	ImGui::EndDisabled();
	ImGui::BeginDisabled(3 != m_iDestructionTriggerKind);
	ImGui::SetNextItemWidth(300.f);
	ImGui::InputText("Receiver collision ID", m_DestructionReceiverId,
		IM_ARRAYSIZE(m_DestructionReceiverId));
	ImGui::EndDisabled();

	const bool_t canCreateBinding =
		!m_SelectedDestructionPatternId.empty() &&
		!m_SelectedDestructionStageId.empty() &&
		!m_DestructionDocument.Get_Mutations().empty();
	ImGui::BeginDisabled(!canCreateBinding);
	if (ImGui::Button("Create Binding for selected pattern stage"))
	{
		DESTRUCTION_BINDING binding;
		binding.bindingId = m_DestructionBindingId;
		binding.mutationId = m_DestructionMutationId;
		binding.patternId = m_SelectedDestructionPatternId;
		binding.stageId = m_SelectedDestructionStageId;
		binding.eTriggerKind =
			0 == m_iDestructionTriggerKind ?
				DESTRUCTION_TRIGGER_KIND::STAGE_ENTER :
			1 == m_iDestructionTriggerKind ?
				DESTRUCTION_TRIGGER_KIND::STAGE_TIME :
			2 == m_iDestructionTriggerKind ?
				DESTRUCTION_TRIGGER_KIND::STAGE_EXIT :
				DESTRUCTION_TRIGGER_KIND::COLLISION_IMPACT;
		binding.iOffsetMs = 1 == m_iDestructionTriggerKind ?
			static_cast<uint32_t>(m_iDestructionOffsetMs) : 0u;
		binding.receiverCollisionId = 3 == m_iDestructionTriggerKind ?
			m_DestructionReceiverId : "";
		binding.isEnabled = false;
		std::string status;
		m_DestructionDocument.Add_Binding(binding, status);
		m_DestructionStatus = status;
	}
	ImGui::EndDisabled();
	if (!canCreateBinding)
	{
		ImGui::TextDisabled(
			"Select a pattern stage above and create a mutation first.");
	}

	if (!ImGui::BeginTable("DestructionBindings", 7,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		return;
	}
	ImGui::TableSetupColumn("bindingId");
	ImGui::TableSetupColumn("pattern");
	ImGui::TableSetupColumn("stage");
	ImGui::TableSetupColumn("trigger");
	ImGui::TableSetupColumn("offset ms");
	ImGui::TableSetupColumn("enabled");
	ImGui::TableSetupColumn("");
	ImGui::TableHeadersRow();
	std::string bindingRemoveRequest;
	for (const DESTRUCTION_BINDING& entry :
		m_DestructionDocument.Get_Bindings())
	{
		ImGui::TableNextRow();
		ImGui::PushID(entry.bindingId.c_str());
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(entry.bindingId.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(entry.patternId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::TextUnformatted(entry.stageId.c_str());
		ImGui::TableSetColumnIndex(3);
		ImGui::TextUnformatted(
			CWorldDestructionDocument::TriggerKind_ToString(
				entry.eTriggerKind));
		ImGui::TableSetColumnIndex(4);
		ImGui::Text("%u", entry.iOffsetMs);
		ImGui::TableSetColumnIndex(5);
		bool_t enabled = entry.isEnabled;
		if (ImGui::Checkbox("##enabled", &enabled))
		{
			DESTRUCTION_BINDING updated = entry;
			updated.isEnabled = enabled;
			std::string status;
			m_DestructionDocument.Update_Binding(updated, status);
			m_DestructionStatus = status;
		}
		ImGui::TableSetColumnIndex(6);
		if (ImGui::SmallButton("Remove"))
			bindingRemoveRequest = entry.bindingId;
		ImGui::PopID();
	}
	ImGui::EndTable();
	if (!bindingRemoveRequest.empty())
		m_DestructionDocument.Remove_Binding(bindingRemoveRequest);
}

void Client::CMapTool::Render_DestructionTimeline()
{
	if (!ImGui::CollapsingHeader("Pattern Timeline",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern || 0u == pattern->iTotalDurationMs)
	{
		ImGui::TextUnformatted(
			"Select a pattern in Encounter Source to see its timeline.");
		return;
	}

	const f32_t width = (std::max)(240.f, ImGui::GetContentRegionAvail().x - 16.f);
	constexpr f32_t barHeight = 26.f;
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	ImDrawList* draw = ImGui::GetWindowDrawList();
	const f32_t totalMs = static_cast<f32_t>(pattern->iTotalDurationMs);

	for (size_t index = 0; index < pattern->stages.size(); ++index)
	{
		const ENCOUNTER_STAGE_REFERENCE& stage = pattern->stages[index];
		const f32_t left = origin.x +
			width * static_cast<f32_t>(stage.iStartOffsetMs) / totalMs;
		const f32_t right = origin.x + width *
			static_cast<f32_t>(stage.iStartOffsetMs + stage.iDurationMs) /
			totalMs;
		const bool_t isActive = "ACTIVE" == stage.stageKind;
		const ImU32 fill = isActive ?
			IM_COL32(190, 90, 60, 210) : IM_COL32(70, 80, 110, 210);
		draw->AddRectFilled(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight), fill);
		draw->AddRect(ImVec2(left, origin.y),
			ImVec2(right - 1.f, origin.y + barHeight),
			IM_COL32(220, 220, 220, 160));
		if (right - left > 44.f)
		{
			draw->AddText(ImVec2(left + 4.f, origin.y + 5.f),
				IM_COL32(240, 240, 240, 255), stage.stageId.c_str());
		}
		if (ImGui::IsMouseHoveringRect(ImVec2(left, origin.y),
			ImVec2(right, origin.y + barHeight)))
		{
			ImGui::SetTooltip("%s | %s | %u ms | tick %u",
				stage.stageId.c_str(), stage.stageKind.c_str(),
				stage.iDurationMs,
				CEncounterPatternReference::To_ServerTick(
					stage.iStartOffsetMs,
					m_EncounterReference.Get_FixedTickHz()));
		}
	}

	/* Draw every binding of this pattern as a marker at its absolute ms so the
	   author sees the requested moment against the real stage boundaries. */
	for (const DESTRUCTION_BINDING& binding :
		m_DestructionDocument.Get_Bindings())
	{
		if (binding.patternId != pattern->patternId)
			continue;
		const auto stage = std::find_if(
			pattern->stages.begin(), pattern->stages.end(),
			[&binding](const ENCOUNTER_STAGE_REFERENCE& value)
			{
				return value.stageId == binding.stageId;
			});
		if (pattern->stages.end() == stage)
			continue;
		uint32_t absoluteMs = stage->iStartOffsetMs;
		if (DESTRUCTION_TRIGGER_KIND::STAGE_TIME == binding.eTriggerKind)
			absoluteMs += (std::min)(binding.iOffsetMs, stage->iDurationMs);
		else if (DESTRUCTION_TRIGGER_KIND::STAGE_EXIT == binding.eTriggerKind)
			absoluteMs += stage->iDurationMs;
		const f32_t markerX = origin.x +
			width * static_cast<f32_t>(absoluteMs) / totalMs;
		const ImU32 color = binding.isEnabled ?
			IM_COL32(120, 255, 120, 255) : IM_COL32(255, 220, 90, 255);
		draw->AddLine(ImVec2(markerX, origin.y - 4.f),
			ImVec2(markerX, origin.y + barHeight + 4.f), color, 2.f);
	}

	ImGui::Dummy(ImVec2(width, barHeight + 10.f));
	ImGui::Text("total %u ms | %u Hz | green marker = enabled binding",
		pattern->iTotalDurationMs,
		m_EncounterReference.Get_FixedTickHz());

	ImGui::TextUnformatted("Stage for the next binding");
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
	{
		if (ImGui::RadioButton(stage.stageId.c_str(),
			m_SelectedDestructionStageId == stage.stageId))
		{
			m_SelectedDestructionStageId = stage.stageId;
			m_iDestructionOffsetMs = 0;
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

void Client::CMapTool::Render_WorldDestructionPanel(bool_t isAssetTest)
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	ImGui::Text("Area: %s",
		nullptr != descriptor ? descriptor->label.c_str() : "NONE");
	ImGui::SameLine();
	ImGui::TextUnformatted(
		"| Saves Data authoring only. The Server owns the real state.");
	ImGui::Separator();

	if (!isAssetTest)
	{
		ImGui::TextUnformatted(
			"Enter the Debug Map Editor workspace to inspect this Area.");
		return;
	}
	if (nullptr == descriptor || descriptor->encounterReference.empty())
	{
		ImGui::TextUnformatted(
			"This Area declares no destruction authoring source.");
		ImGui::TextUnformatted("Select Valtan in the workspace bar.");
		return;
	}

	if (ImGui::RadioButton("Easy Wall Editor", !m_bDestructionAdvancedMode))
		m_bDestructionAdvancedMode = false;
	ImGui::SameLine();
	if (ImGui::RadioButton("Advanced Graph Editor", m_bDestructionAdvancedMode))
		m_bDestructionAdvancedMode = true;
	ImGui::Separator();
	if (!m_bDestructionAdvancedMode)
	{
		Render_DestructionSimpleEditor();
		return;
	}

	Render_DestructionEncounterSource();
	Render_DestructionTimeline();
	Render_DestructionGroupEditor();
	Render_DestructionBindingEditor();
	Render_DestructionDeployList();
	Render_DestructionWorldRows();
	Render_DestructionNavigationRegions();
	Render_DestructionDiagnostics();
}

void Client::CMapTool::Render_DestructionSimulationWindow(
	const bool_t isAssetTest)
{
	if (!m_bOpen || !isAssetTest ||
		TOOL_MODE::WORLD_DESTRUCTION != m_eToolMode)
	{
		return;
	}
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->destructionSimulationDocument.empty())
		return;

	ImGui::SetNextWindowPos(ImVec2(450.f, 35.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(820.f, 760.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.f);
	if (!ImGui::Begin("Destruction Model View"))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Area %s | %zu profiles",
		descriptor->areaId.c_str(),
		m_DestructionSimulationDocument.Get_Profiles().size());
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Authoring preview only; Server state and collision are unchanged");
	ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.22f, 1.f),
		"PROJECT_AUTHORED destruction: %u PhysX pieces per wall",
		CDestructionSimulationRuntime::PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT);
	ImGui::TextWrapped(
		"DEPLOY_ITR_02316 uses 12 macro shards derived from every triangle of "
		"its exact fractured wall mesh. Other Deploy assets use the four generic "
		"Valtan stone fallbacks. Original chunk pivots and source particle %s "
		"were not recovered, so the flight remains PROJECT_AUTHORED.",
		CDestructionSimulationRuntime::PROJECT_AUTHORED_SOURCE_PARTICLE_ID);
	if (m_bDestructionDebrisPrototypesReady)
	{
		ImGui::TextColored(ImVec4(0.35f, 0.9f, 0.45f, 1.f), "%s",
			m_DestructionDebrisPrototypeStatus.c_str());
	}
	else
	{
		ImGui::TextColored(ImVec4(1.f, 0.35f, 0.3f, 1.f), "%s",
			m_DestructionDebrisPrototypeStatus.c_str());
	}
	ImGui::BeginDisabled(!m_DestructionSimulationDocument.Is_Dirty() ||
		m_bDestructionSimulationElementDraftDirty);
	if (ImGui::Button("Save Simulations"))
		Save_AllAuthoring();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Reload Simulations"))
		Load_DestructionSimulation();
	ImGui::SameLine();
	const bool_t hasSelectedDeployGroup =
		nullptr != m_DestructionDocument.Find_Group(
			m_SelectedDestructionGroupId);
	ImGui::BeginDisabled(!hasSelectedDeployGroup);
	if (ImGui::Button("Create Default for Selected Group"))
		Create_DefaultDestructionSimulationProfile();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_DestructionSimulationDocument.Is_Dirty() ? "[unsaved]" : "[saved]");
	ImGui::TextWrapped("%s", m_DestructionSimulationStatus.c_str());

	Render_DestructionSimulationTimeline();
	ImGui::Separator();
	if (ImGui::BeginTable("DestructionSimulationColumns", 2,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Emitters / Fragments", ImGuiTableColumnFlags_WidthStretch,
			0.43f);
		ImGui::TableSetupColumn("Emitter / Fragment Detail", ImGuiTableColumnFlags_WidthStretch,
			0.57f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_DestructionSimulationOutliner();
		ImGui::TableSetColumnIndex(1);
		Render_DestructionSimulationDetail();
		ImGui::EndTable();
	}
	ImGui::End();
}

void Client::CMapTool::Render_DestructionSimulationTimeline()
{
	ImGui::SeparatorText("Physics Timeline");
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		Get_SelectedDestructionSimulationProfile();
	const bool_t hasProfile = nullptr != profile;
	const DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT snapshot =
		nullptr != m_pDestructionSimulationController ?
			m_pDestructionSimulationController->Get_Snapshot() :
			DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT{};

	ImGui::BeginDisabled(!hasProfile);
	if (ImGui::Button("Stage Selected (Paused)"))
		Request_StageDestructionSimulation(*profile, false, false);
	ImGui::SameLine();
	if (ImGui::Button("Play All Fragments"))
	{
		if (Request_StageDestructionSimulation(*profile, false, true))
		{
			m_pDestructionSimulationController->Request_SetScope(
				DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
		}
	}
	ImGui::SameLine();
	const bool_t isPlaying =
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING == snapshot.eState;
	if (ImGui::Button(isPlaying ? "Pause" : "Play"))
	{
		if (isPlaying)
			m_pDestructionSimulationController->Request_Pause();
		else if (DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED == snapshot.eState)
		{
			m_pDestructionSimulationController->Request_Reset();
			m_pDestructionSimulationController->Request_Play();
		}
		else if (m_pDestructionSimulationController->Get_Runtime().Is_Staged())
			m_pDestructionSimulationController->Request_Play();
		else
			Request_StageDestructionSimulation(*profile, false, true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Restart + Play"))
	{
		if (!m_pDestructionSimulationController->Get_Runtime().Is_Staged())
			Request_StageDestructionSimulation(*profile, false, true);
		else
		{
			m_pDestructionSimulationController->Request_Reset();
			m_pDestructionSimulationController->Request_Play();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Single 60 Hz Step"))
		m_pDestructionSimulationController->Request_SingleStep();
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_bDestructionSimulationLoop);

	const bool_t allDebris =
		DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == snapshot.eScope;
	const bool_t soloEmitter =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == snapshot.eScope;
	const bool_t soloFragment =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == snapshot.eScope;
	if (ImGui::RadioButton("All Fragments", allDebris))
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
	}
	ImGui::SameLine();
	const bool_t canSoloEmitter =
		!m_SelectedDestructionSimulationElementId.empty();
	ImGui::BeginDisabled(!canSoloEmitter);
	if (ImGui::RadioButton("Solo Emitter", soloEmitter))
	{
		if (hasProfile && AreEmittersAuthoredAsOneWall(*profile))
		{
			m_pDestructionSimulationController->Request_SetScope(
				DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
		}
		else
		{
			m_pDestructionSimulationController->Request_SetScope(
				DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
				m_SelectedDestructionSimulationElementId);
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool_t canSoloFragment =
		hasProfile && !m_SelectedDestructionSimulationFragmentId.empty();
	ImGui::BeginDisabled(!canSoloFragment);
	if (ImGui::RadioButton("Solo Fragment", soloFragment))
	{
		m_pDestructionSimulationController->Request_SetScope(
			DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
			m_SelectedDestructionSimulationFragmentId);
	}
	ImGui::EndDisabled();

	size_t fragmentCount = hasProfile ?
		profile->Elements.size() *
		CDestructionSimulationRuntime::PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT :
		0u;
	if (nullptr != m_pDestructionSimulationController)
	{
		const DESTRUCTION_SIMULATION_FRAME& frame =
			m_pDestructionSimulationController->Get_Runtime().Get_Frame();
		if (hasProfile && frame.profileId == profile->profileId)
		{
			fragmentCount = 0u;
			for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME& elementFrame :
				frame.Elements)
			{
				fragmentCount += elementFrame.Fragments.size();
			}
		}
	}
	ImGui::TextDisabled("Scope %s | %zu mesh emitters | %zu fragments",
		SimulationScopeLabel(snapshot.eScope),
		hasProfile ? profile->Elements.size() : 0u,
		fragmentCount);

	ImGui::Text("%s | %.3f / %.3f s | fixed %.3f ms",
		SimulationPlaybackStateLabel(snapshot.eState),
		snapshot.fSampleTimeSeconds,
		hasProfile ? profile->fDurationSeconds : snapshot.fDurationSeconds,
		CDestructionSimulationController::FIXED_DELTA_SECONDS * 1000.f);
	if (hasProfile)
	{
		f32_t sampleTime = (std::clamp)(snapshot.fSampleTimeSeconds,
			0.f, profile->fDurationSeconds);
		if (ImGui::SliderFloat("Sample Time", &sampleTime,
			0.f, profile->fDurationSeconds, "%.3f s"))
		{
			if (!m_pDestructionSimulationController->Get_Runtime().Is_Staged())
				Request_StageDestructionSimulation(*profile, false, false);
			m_pDestructionSimulationController->Request_Seek(sampleTime);
		}

		const f32_t width = (std::max)(240.f,
			ImGui::GetContentRegionAvail().x - 12.f);
		constexpr f32_t barHeight = 18.f;
		const ImVec2 origin = ImGui::GetCursorScreenPos();
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(origin,
			ImVec2(origin.x + width, origin.y + barHeight),
			IM_COL32(45, 52, 68, 210));
		for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile->Elements)
		{
			const f32_t triggerTime =
				DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
					element.Trigger.eKind ? element.Trigger.fTimeSeconds : 0.f;
			const f32_t markerX = origin.x + width *
				triggerTime / profile->fDurationSeconds;
			const ImU32 markerColor =
				DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT ==
					element.Trigger.eKind ? IM_COL32(255, 95, 95, 255) :
				IM_COL32(120, 235, 150, 255);
			draw->AddLine(ImVec2(markerX, origin.y - 2.f),
				ImVec2(markerX, origin.y + barHeight + 2.f), markerColor, 2.f);
		}
		draw->AddLine(
			ImVec2(origin.x + width * sampleTime / profile->fDurationSeconds,
				origin.y - 4.f),
			ImVec2(origin.x + width * sampleTime / profile->fDurationSeconds,
				origin.y + barHeight + 4.f),
			IM_COL32(255, 220, 80, 255), 2.f);
		ImGui::Dummy(ImVec2(width, barHeight + 6.f));

		std::unordered_set<std::string> collisionReceivers;
		for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile->Elements)
		{
			if (DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT ==
				element.Trigger.eKind &&
				!element.Trigger.receiverCollisionId.empty())
			{
				collisionReceivers.insert(element.Trigger.receiverCollisionId);
			}
		}
		for (const std::string& receiver : collisionReceivers)
		{
			ImGui::PushID(receiver.c_str());
			if (ImGui::SmallButton(("Fire Collision: " + receiver).c_str()))
				m_pDestructionSimulationController->Request_Collision(receiver);
			ImGui::PopID();
		}
	}
	ImGui::EndDisabled();
	if (nullptr != m_pDestructionSimulationController)
	{
		ImGui::TextDisabled("%s",
			m_pDestructionSimulationController->Get_Snapshot().status.c_str());
	}
}

void Client::CMapTool::Render_DestructionSimulationOutliner()
{
	ImGui::SeparatorText("Mesh Emitters / Fragments");
	ImGui::InputText("Filter", m_DestructionSimulationFilter,
		IM_ARRAYSIZE(m_DestructionSimulationFilter));
	const std::string filter = m_DestructionSimulationFilter;
	const DESTRUCTION_SIMULATION_FRAME* frame =
		nullptr != m_pDestructionSimulationController ?
			&m_pDestructionSimulationController->Get_Runtime().Get_Frame() : nullptr;

	if (ImGui::BeginChild("DestructionProfiles", ImVec2(0.f, 420.f), true))
	{
		for (const DESTRUCTION_SIMULATION_PROFILE& profile :
			m_DestructionSimulationDocument.Get_Profiles())
		{
			const bool_t profileSelected =
				profile.profileId == m_SelectedDestructionSimulationProfileId;
			const std::string profileLabel = profile.profileId + "##profile";
			const bool_t profileOpen = ImGui::TreeNodeEx(profileLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow |
				(profileSelected ? ImGuiTreeNodeFlags_DefaultOpen : 0) |
				(profileSelected ? ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				Select_DestructionSimulationProfile(profile.profileId);
				Refresh_DestructionHighlight();
			}
			if (!profileOpen)
				continue;

			ImGui::TextDisabled("group %s | %.2f s | %zu mesh emitters",
				profile.groupId.c_str(), profile.fDurationSeconds,
				profile.Elements.size());
			const bool_t linkedEmitters = AreEmittersAuthoredAsOneWall(profile);
			for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile.Elements)
			{
				/* Linked walls render once, under the first element's identity. */
				if (linkedEmitters && &element != &profile.Elements.front())
					continue;
				const DESTRUCTION_SIMULATION_ELEMENT_FRAME* runtimeElement = nullptr;
				if (nullptr != frame && frame->profileId == profile.profileId)
				{
					const auto found = std::find_if(
						frame->Elements.begin(), frame->Elements.end(),
						[&element](const DESTRUCTION_SIMULATION_ELEMENT_FRAME& value)
						{
							return value.elementId == element.elementId;
						});
					if (found != frame->Elements.end())
						runtimeElement = &*found;
				}

				bool_t matches = filter.empty() ||
					MatchesFilter(element.elementId, filter.c_str());
				if (!matches && nullptr != runtimeElement)
				{
					matches = std::any_of(
						runtimeElement->Fragments.begin(),
						runtimeElement->Fragments.end(),
						[&filter](const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& fragment)
						{
							return MatchesFilter(fragment.fragmentId, filter.c_str()) ||
								MatchesFilter(fragment.modelAssetId, filter.c_str());
						});
				}
				if (!matches)
					continue;

				ImGui::PushID(element.elementId.c_str());
				const bool_t elementSelected = profileSelected &&
					element.elementId == m_SelectedDestructionSimulationElementId;
				size_t linkedFragmentCount = 0u;
				if (linkedEmitters && nullptr != frame &&
					frame->profileId == profile.profileId)
				{
					for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME& linkedFrame :
						frame->Elements)
					{
						linkedFragmentCount += linkedFrame.Fragments.size();
					}
				}
				std::ostringstream emitterLabel;
				if (linkedEmitters)
				{
					emitterLabel << "Wall Emitters ("
						<< profile.Elements.size() << " walls";
					if (0u != linkedFragmentCount)
						emitterLabel << ": " << linkedFragmentCount;
					emitterLabel << ")";
				}
				else
				{
					emitterLabel << element.elementId << " (Emitter";
					if (nullptr != runtimeElement)
						emitterLabel << ": " << runtimeElement->Fragments.size();
					emitterLabel << ")";
				}
				emitterLabel << "##emitter";
				ImGuiTreeNodeFlags emitterFlags =
					ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_SpanAvailWidth;
				if (elementSelected)
					emitterFlags |= ImGuiTreeNodeFlags_Selected |
						ImGuiTreeNodeFlags_DefaultOpen;
				if (!filter.empty())
					emitterFlags |= ImGuiTreeNodeFlags_DefaultOpen;
				const bool_t emitterOpen = ImGui::TreeNodeEx(
					emitterLabel.str().c_str(), emitterFlags);
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				{
					if (!profileSelected)
						Select_DestructionSimulationProfile(profile.profileId);
					Select_DestructionSimulationElement(element.elementId);
				}
				if (ImGui::IsItemHovered())
				{
					if (linkedEmitters)
					{
						ImGui::SetTooltip(
							"%zu linked walls authored as one\n"
							"velocity = direction x %.3f m/s",
							profile.Elements.size(),
							element.fSpeedMetersPerSecond);
					}
					else
					{
						ImGui::SetTooltip(
							"Deploy %llu\nvelocity = direction x %.3f m/s",
							static_cast<unsigned long long>(
								element.sourceRuntimePlacementId),
							element.fSpeedMetersPerSecond);
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(linkedEmitters ?
					"Play Linked Walls" : "Solo Emitter + Play"))
				{
					if (!profileSelected)
						Select_DestructionSimulationProfile(profile.profileId);
					Select_DestructionSimulationElement(element.elementId);
					const DESTRUCTION_SIMULATION_PROFILE* selected =
						Get_SelectedDestructionSimulationProfile();
					if (nullptr != selected)
					{
						const DESTRUCTION_SIMULATION_FRAME& activeFrame =
							m_pDestructionSimulationController->Get_Runtime().Get_Frame();
						const bool_t alreadyStaged =
							m_pDestructionSimulationController->Get_Runtime().Is_Staged() &&
							activeFrame.profileId == selected->profileId;
						if (alreadyStaged || Request_StageDestructionSimulation(
							*selected, false, true))
						{
							if (linkedEmitters)
							{
								m_pDestructionSimulationController->Request_SetScope(
									DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
							}
							else
							{
								m_pDestructionSimulationController->Request_SetScope(
									DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
									element.elementId);
							}
							m_pDestructionSimulationController->Request_Reset();
							m_pDestructionSimulationController->Request_Play();
							m_DestructionSimulationStatus = linkedEmitters ?
								"Linked wall audition requested: " + profile.profileId :
								"Solo mesh emitter audition requested: " +
									element.elementId;
						}
					}
				}

				if (emitterOpen)
				{
					if (nullptr == runtimeElement)
					{
						ImGui::TextDisabled(
							"Stage this profile to materialize its 12 fragment rows.");
					}
					else
					{
						std::vector<const DESTRUCTION_SIMULATION_ELEMENT_FRAME*>
							ownerFrames;
						if (linkedEmitters)
						{
							ownerFrames.reserve(frame->Elements.size());
							for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME& linkedFrame :
								frame->Elements)
							{
								ownerFrames.push_back(&linkedFrame);
							}
						}
						else
						{
							ownerFrames.push_back(runtimeElement);
						}
						for (const DESTRUCTION_SIMULATION_ELEMENT_FRAME* ownerFrame :
							ownerFrames)
						{
						ImGui::TextDisabled(
								"Deploy %llu | %s | life %.2f",
								static_cast<unsigned long long>(
									ownerFrame->sourceRuntimePlacementId),
								SimulationElementStateLabel(ownerFrame->eState),
								ownerFrame->fNormalizedLife);
						for (const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& fragment :
							ownerFrame->Fragments)
						{
							if (!filter.empty() &&
								!MatchesFilter(fragment.fragmentId, filter.c_str()) &&
								!MatchesFilter(fragment.modelAssetId, filter.c_str()))
							{
								continue;
							}
							ImGui::PushID(fragment.fragmentId.c_str());
							const bool_t fragmentSelected = profileSelected &&
								fragment.fragmentId ==
									m_SelectedDestructionSimulationFragmentId;
							std::ostringstream fragmentLabel;
							fragmentLabel << fragment.fragmentId << " ["
								<< SimulationElementStateLabel(fragment.eState)
								<< " | life " << std::fixed << std::setprecision(2)
								<< fragment.fNormalizedLife << "]";
							const f32_t soloWidth = 88.f;
							const f32_t rowWidth = (std::max)(1.f,
								ImGui::GetContentRegionAvail().x - soloWidth);
							if (ImGui::Selectable(fragmentLabel.str().c_str(),
								fragmentSelected, 0, ImVec2(rowWidth, 0.f)))
							{
								Select_DestructionSimulationFragment(
									ownerFrame->elementId, fragment.fragmentId);
							}
							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip("Piece %u\n%s",
									fragment.pieceIndex,
									fragment.modelAssetId.c_str());
							}
							ImGui::SameLine();
							if (ImGui::SmallButton("Solo + Play"))
							{
								Select_DestructionSimulationFragment(
									ownerFrame->elementId, fragment.fragmentId);
								if (m_SelectedDestructionSimulationFragmentId ==
									fragment.fragmentId)
								{
									m_pDestructionSimulationController->Request_SetScope(
										DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
										fragment.fragmentId);
									m_pDestructionSimulationController->Request_Reset();
									m_pDestructionSimulationController->Request_Play();
									m_DestructionSimulationStatus =
										"Solo fragment audition requested: " +
										fragment.fragmentId;
								}
							}
							ImGui::PopID();
						}
						}
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}
	ImGui::EndChild();
	ImGui::TextDisabled(
		"Profile = complete destruction; source wall = one 12-piece mesh emitter");
}

void Client::CMapTool::Render_DestructionSimulationDetail()
{
	ImGui::SeparatorText("Emitter / Fragment Detail");
	const DESTRUCTION_SIMULATION_PROFILE* profile =
		Get_SelectedDestructionSimulationProfile();
	if (nullptr == profile ||
		!m_DestructionSimulationElementDraft.has_value())
	{
		ImGui::TextDisabled("Select one debris element in All Debris.");
		return;
	}

	DESTRUCTION_SIMULATION_ELEMENT& draft =
		*m_DestructionSimulationElementDraft;
	if (!m_SelectedDestructionSimulationFragmentId.empty())
	{
		const DESTRUCTION_SIMULATION_FRAGMENT_FRAME* selectedFragment = nullptr;
		if (nullptr != m_pDestructionSimulationController)
		{
			const DESTRUCTION_SIMULATION_FRAME& frame =
				m_pDestructionSimulationController->Get_Runtime().Get_Frame();
			if (frame.profileId == profile->profileId)
			{
				const auto runtimeElement = std::find_if(
					frame.Elements.begin(), frame.Elements.end(),
					[this](const DESTRUCTION_SIMULATION_ELEMENT_FRAME& value)
					{
						return value.elementId ==
							m_SelectedDestructionSimulationElementId;
					});
				if (runtimeElement != frame.Elements.end())
				{
					const auto fragment = std::find_if(
						runtimeElement->Fragments.begin(),
						runtimeElement->Fragments.end(),
						[this](const DESTRUCTION_SIMULATION_FRAGMENT_FRAME& value)
						{
							return value.fragmentId ==
								m_SelectedDestructionSimulationFragmentId;
						});
					if (fragment != runtimeElement->Fragments.end())
						selectedFragment = &*fragment;
				}
			}
		}

		ImGui::SeparatorText("Selected Fragment (Read-only Runtime Sample)");
		if (nullptr == selectedFragment)
		{
			ImGui::TextDisabled(
				"The selected fragment is unavailable. Stage the selected profile again.");
		}
		else
		{
			const f32_t speed = std::sqrt(
				selectedFragment->vLinearVelocity.x *
					selectedFragment->vLinearVelocity.x +
				selectedFragment->vLinearVelocity.y *
					selectedFragment->vLinearVelocity.y +
				selectedFragment->vLinearVelocity.z *
					selectedFragment->vLinearVelocity.z);
			ImGui::TextWrapped("Stable ID: %s",
				selectedFragment->fragmentId.c_str());
			ImGui::Text("Piece index: %u", selectedFragment->pieceIndex);
			ImGui::TextWrapped("Model: %s",
				selectedFragment->modelAssetId.c_str());
			ImGui::Text("State: %s | life %.3f",
				SimulationElementStateLabel(selectedFragment->eState),
				selectedFragment->fNormalizedLife);
			ImGui::Text("Position: %.3f, %.3f, %.3f",
				selectedFragment->vWorldPosition.x,
				selectedFragment->vWorldPosition.y,
				selectedFragment->vWorldPosition.z);
			ImGui::Text("Velocity: %.3f, %.3f, %.3f m/s | speed %.3f",
				selectedFragment->vLinearVelocity.x,
				selectedFragment->vLinearVelocity.y,
				selectedFragment->vLinearVelocity.z,
				speed);
			if (ImGui::Button("Solo + Play Selected Fragment"))
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT,
					selectedFragment->fragmentId);
				m_pDestructionSimulationController->Request_Reset();
				m_pDestructionSimulationController->Request_Play();
				m_DestructionSimulationStatus =
					"Solo fragment audition requested: " +
					selectedFragment->fragmentId;
			}
			ImGui::TextDisabled(
				"Fragment values are generated deterministically from the emitter authoring below.");
		}
	}

	const bool_t linkedEmitters = AreEmittersAuthoredAsOneWall(*profile);
	if (linkedEmitters)
	{
		ImGui::SeparatorText(
			"Emitter Authoring (applies to every linked wall)");
		ImGui::TextWrapped("%zu walls authored as one emitter",
			profile->Elements.size());
		for (const DESTRUCTION_SIMULATION_ELEMENT& linked : profile->Elements)
		{
			ImGui::BulletText("%s | Deploy %llu",
				linked.elementId.c_str(),
				static_cast<unsigned long long>(
					linked.sourceRuntimePlacementId));
		}
	}
	else
	{
		ImGui::SeparatorText(
			"Emitter Authoring (applies to all 12 fragments)");
		ImGui::TextWrapped("%s", draft.elementId.c_str());
		ImGui::Text("Source Deploy placement: %llu",
			static_cast<unsigned long long>(draft.sourceRuntimePlacementId));
	}
	ImGui::TextDisabled(
		"These are authored simulation semantics. PhysX receives the derived velocity and gravity policy.");

	bool_t changed = false;
	changed |= ImGui::DragFloat3("Spawn Offset", &draft.vSpawnOffset.x,
		0.01f, -CDestructionSimulationDocument::MAX_ABSOLUTE_OFFSET,
		CDestructionSimulationDocument::MAX_ABSOLUTE_OFFSET, "%.3f");
	float3_t editedDirection = draft.vDirection;
	const bool_t directionChanged = ImGui::DragFloat3(
		"Direction (World Unit)", &editedDirection.x,
		0.01f, -1.f, 1.f, "%.4f");
	if (directionChanged)
	{
		if (!NormalizeSimulationDirection(editedDirection))
		{
			m_DestructionSimulationStatus =
				"Direction cannot be zero; the previous preview was preserved";
		}
		else
		{
			draft.vDirection = editedDirection;
			changed = true;
		}
	}
	changed |= ImGui::DragFloat("Speed (m/s)",
		&draft.fSpeedMetersPerSecond, 0.05f, 0.f,
		CDestructionSimulationDocument::MAX_SPEED_METERS_PER_SECOND,
		"%.3f", ImGuiSliderFlags_AlwaysClamp);
	changed |= ImGui::DragFloat("Gravity Scale",
		&draft.fGravityScale, 0.01f, 0.f,
		CDestructionSimulationDocument::MAX_GRAVITY_SCALE,
		"%.3f", ImGuiSliderFlags_AlwaysClamp);
	f32_t maximumLifetime = profile->fDurationSeconds;
	if (DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
		draft.Trigger.eKind)
	{
		maximumLifetime = (std::max)(
			CDestructionSimulationController::FIXED_DELTA_SECONDS,
			profile->fDurationSeconds - draft.Trigger.fTimeSeconds);
	}
	changed |= ImGui::DragFloat("Lifetime (s)",
		&draft.fLifetimeSeconds, 0.01f,
		CDestructionSimulationController::FIXED_DELTA_SECONDS,
		maximumLifetime, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);

	const float3_t velocity = {
		draft.vDirection.x * draft.fSpeedMetersPerSecond,
		draft.vDirection.y * draft.fSpeedMetersPerSecond,
		draft.vDirection.z * draft.fSpeedMetersPerSecond
	};
	ImGui::TextDisabled("Derived velocity: %.3f, %.3f, %.3f m/s",
		velocity.x, velocity.y, velocity.z);

	const char_t* triggerLabels[] = {
		"Immediate", "Timeline Time", "Collision Impact" };
	int32_t triggerKind =
		static_cast<int32_t>(draft.Trigger.eKind);
	if (ImGui::Combo("Trigger Condition", &triggerKind,
		triggerLabels, IM_ARRAYSIZE(triggerLabels)))
	{
		draft.Trigger.eKind =
			static_cast<DESTRUCTION_SIMULATION_TRIGGER_KIND>(triggerKind);
		if (DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME !=
			draft.Trigger.eKind)
		{
			draft.Trigger.fTimeSeconds = 0.f;
		}
		if (DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT !=
			draft.Trigger.eKind)
		{
			draft.Trigger.receiverCollisionId.clear();
			m_DestructionSimulationReceiverId[0] = '\0';
		}
		changed = true;
	}
	if (DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
		draft.Trigger.eKind)
	{
		const f32_t maximumTriggerTime = (std::max)(0.f,
			profile->fDurationSeconds - draft.fLifetimeSeconds);
		changed |= ImGui::DragFloat("Trigger Time (s)",
			&draft.Trigger.fTimeSeconds, 0.01f, 0.f,
			maximumTriggerTime, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
	}
	else if (DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT ==
		draft.Trigger.eKind)
	{
		if (ImGui::InputText("Receiver Collision ID",
			m_DestructionSimulationReceiverId,
			IM_ARRAYSIZE(m_DestructionSimulationReceiverId)))
		{
			draft.Trigger.receiverCollisionId =
				m_DestructionSimulationReceiverId;
			changed = true;
		}
		if (!draft.Trigger.receiverCollisionId.empty() &&
			ImGui::Button("Fire This Collision"))
		{
			m_pDestructionSimulationController->Request_Collision(
				draft.Trigger.receiverCollisionId);
		}
	}

	if (changed)
	{
		m_bDestructionSimulationElementDraftDirty = true;
		m_DestructionSimulationStatus =
			"Live Detail draft staged; Apply commits it to the authoring document";
		Stage_DestructionElementDraftPreview();
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!m_bDestructionSimulationElementDraftDirty);
	if (ImGui::Button("Apply Detail"))
	{
		std::string status;
		const bool_t wasPlaying =
			DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING ==
				m_pDestructionSimulationController->Get_Snapshot().eState;
		/* Copy the identities first: Update_Element mutates the document that
		   `profile` points into. Each linked wall keeps its own elementId,
		   source placement and suppression aliases; only the authored
		   simulation values are shared. */
		bool_t committed = false;
		if (linkedEmitters)
		{
			std::vector<DESTRUCTION_SIMULATION_ELEMENT> linkedElements =
				profile->Elements;
			committed = true;
			for (DESTRUCTION_SIMULATION_ELEMENT& linked : linkedElements)
			{
				linked.vSpawnOffset = draft.vSpawnOffset;
				linked.vDirection = draft.vDirection;
				linked.fSpeedMetersPerSecond = draft.fSpeedMetersPerSecond;
				linked.fGravityScale = draft.fGravityScale;
				linked.fLifetimeSeconds = draft.fLifetimeSeconds;
				linked.Trigger = draft.Trigger;
				if (!m_DestructionSimulationDocument.Update_Element(
					m_SelectedDestructionSimulationProfileId, linked, status))
				{
					committed = false;
					break;
				}
			}
		}
		else
		{
			committed = m_DestructionSimulationDocument.Update_Element(
				m_SelectedDestructionSimulationProfileId, draft, status);
		}
		if (committed)
		{
			m_bDestructionSimulationElementDraftDirty = false;
			m_DestructionSimulationStatus = status + "; Save required";
			const DESTRUCTION_SIMULATION_PROFILE* committed =
				Get_SelectedDestructionSimulationProfile();
			if (nullptr != committed)
				Request_StageDestructionSimulation(
					*committed, true, wasPlaying);
		}
		else
		{
			m_DestructionSimulationStatus = status;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert Detail"))
	{
		const DESTRUCTION_SIMULATION_ELEMENT* saved =
			m_DestructionSimulationDocument.Find_Element(
				m_SelectedDestructionSimulationProfileId,
				m_SelectedDestructionSimulationElementId);
		if (nullptr != saved)
		{
			m_DestructionSimulationElementDraft = *saved;
			m_bDestructionSimulationElementDraftDirty = false;
			strncpy_s(m_DestructionSimulationReceiverId,
				saved->Trigger.receiverCollisionId.c_str(), _TRUNCATE);
			Request_StageDestructionSimulation(*profile, true, false);
			m_DestructionSimulationStatus =
				"Reverted debris Detail to the saved authoring value";
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button(linkedEmitters ?
		"Play Linked Walls" : "Solo Emitter + Play"))
	{
		const bool_t staged = m_bDestructionSimulationElementDraftDirty ?
			Stage_DestructionElementDraftPreview() :
			Request_StageDestructionSimulation(*profile, false, true);
		if (staged)
		{
			if (linkedEmitters)
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS);
			}
			else
			{
				m_pDestructionSimulationController->Request_SetScope(
					DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED,
					draft.elementId);
			}
			m_pDestructionSimulationController->Request_Reset();
			m_pDestructionSimulationController->Request_Play();
		}
	}
	if (m_bDestructionSimulationElementDraftDirty)
		ImGui::TextDisabled("Detail draft is local until Apply Detail.");
}

void Client::CMapTool::Render_DestructionEncounterSource()
{
	if (!ImGui::CollapsingHeader("Encounter Source",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	if (ImGui::Button("Reload Encounter Reference"))
		Load_EncounterReference();
	ImGui::SameLine();
	ImGui::TextWrapped("%s", m_EncounterReferenceStatus.c_str());
	if (!m_EncounterReference.Is_Ready())
		return;

	const uint32_t fixedTickHz = m_EncounterReference.Get_FixedTickHz();
	ImGui::Text("Encounter %s | Boss %s | %u Hz | %zu patterns",
		m_EncounterReference.Get_EncounterId().c_str(),
		m_EncounterReference.Get_BossArchetypeId().c_str(),
		fixedTickHz,
		m_EncounterReference.Get_Patterns().size());

	/* The boss placement of this Area must reference the same encounter this
	   reference document declares, otherwise the two authoring layers drifted. */
	const WORLD_GAMEPLAY_PLACEMENT* bossPlacement = nullptr;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		m_WorldGameplayDocument.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::BOSS == placement.eKind)
		{
			bossPlacement = &placement;
			break;
		}
	}
	if (nullptr == bossPlacement)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"Gameplay document has no boss placement to cross-check.");
	}
	else if (bossPlacement->encounterId !=
		m_EncounterReference.Get_EncounterId())
	{
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f),
			"Boss placement %s references %s, not %s.",
			bossPlacement->placementId.c_str(),
			bossPlacement->encounterId.c_str(),
			m_EncounterReference.Get_EncounterId().c_str());
	}

	if (ImGui::BeginTable("DestructionPatterns", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 180.f)))
	{
		ImGui::TableSetupColumn("patternId");
		ImGui::TableSetupColumn("HP bar");
		ImGui::TableSetupColumn("stages");
		ImGui::TableSetupColumn("total ms");
		ImGui::TableHeadersRow();
		for (const ENCOUNTER_PATTERN_REFERENCE& entry :
			m_EncounterReference.Get_Patterns())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t isSelected =
				m_SelectedDestructionPatternId == entry.patternId;
			if (ImGui::Selectable(entry.patternId.c_str(), isSelected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedDestructionPatternId = entry.patternId;
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%u", entry.iTriggerHealthBar);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%zu", entry.stages.size());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", entry.iTotalDurationMs);
		}
		ImGui::EndTable();
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern)
	{
		ImGui::TextUnformatted("Select a pattern to inspect its stages.");
		return;
	}

	ImGui::Text("%s | actionId %s | selection %s",
		pattern->displayName.c_str(),
		pattern->actionId.c_str(),
		pattern->selectionMode.c_str());
	if (ImGui::BeginTable("DestructionStages", 7,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("stageId");
		ImGui::TableSetupColumn("kind");
		ImGui::TableSetupColumn("start ms");
		ImGui::TableSetupColumn("duration ms");
		ImGui::TableSetupColumn("start tick");
		ImGui::TableSetupColumn("hitShape");
		ImGui::TableSetupColumn("actionId");
		ImGui::TableHeadersRow();
		for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(stage.stageId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(stage.stageKind.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%u", stage.iStartOffsetMs);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", stage.iDurationMs);
			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%u", CEncounterPatternReference::To_ServerTick(
				stage.iStartOffsetMs, fixedTickHz));
			ImGui::TableSetColumnIndex(5);
			ImGui::TextUnformatted(stage.hitShape.c_str());
			ImGui::TableSetColumnIndex(6);
			ImGui::TextUnformatted(stage.actionId.c_str());
		}
		ImGui::EndTable();
	}
}

void Client::CMapTool::Render_DestructionDeployList()
{
	if (!ImGui::CollapsingHeader("Deploy Props",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	const std::vector<DEPLOY_RUNTIME_ENTRY>& entries =
		m_DeployRuntime.Get_Entries();
	ImGui::Text("Loaded placements: %zu | Runtime: %s",
		entries.size(),
		m_DeployRuntime.Is_Loaded() ? "READY" : "NOT LOADED");
	ImGui::SetNextItemWidth(220.f);
	ImGui::InputText("Asset filter", m_DestructionDeployFilter,
		IM_ARRAYSIZE(m_DestructionDeployFilter));
	ImGui::SameLine();
	ImGui::Checkbox("Only rows with stateOffActionId",
		&m_bDestructionOnlyWithOffAction);

	if (!ImGui::BeginTable("DestructionDeployRows", 6,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 240.f)))
	{
		return;
	}
	ImGui::TableSetupColumn("runtimePlacementId");
	ImGui::TableSetupColumn("assetId");
	ImGui::TableSetupColumn("stateOff");
	ImGui::TableSetupColumn("trig");
	ImGui::TableSetupColumn("position");
	ImGui::TableSetupColumn("state");
	ImGui::TableHeadersRow();

	const std::string filter = m_DestructionDeployFilter;
	for (const DEPLOY_RUNTIME_ENTRY& entry : entries)
	{
		if (m_bDestructionOnlyWithOffAction &&
			0u == entry.placement.stateOffActionId)
		{
			continue;
		}
		if (!filter.empty() &&
			std::string::npos == entry.placement.assetId.find(filter))
		{
			continue;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		const std::string idText =
			std::to_string(entry.placement.runtimePlacementId);
		const bool_t isSelected = m_iSelectedDeployPlacementId ==
			entry.placement.runtimePlacementId;
		if (ImGui::Selectable(idText.c_str(), isSelected,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			Select_DestructionWall(
				entry.placement.runtimePlacementId, "advanced deploy list");
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(entry.placement.assetId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%u", entry.placement.stateOffActionId);
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%u", entry.placement.triggerBinaryOccurrenceCount);
		ImGui::TableSetColumnIndex(4);
		ImGui::Text("%.2f, %.2f, %.2f",
			entry.placement.position.x,
			entry.placement.position.y,
			entry.placement.position.z);
		ImGui::TableSetColumnIndex(5);
		ImGui::TextUnformatted(nullptr != entry.object ? "SPAWNED" : "NONE");
	}
	ImGui::EndTable();
}

void Client::CMapTool::Render_DestructionWorldRows()
{
	if (!ImGui::CollapsingHeader("World Gameplay destroyable rows"))
		return;

	size_t destroyableCount = 0;
	if (ImGui::BeginTable("DestructionWorldRows", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("placementId");
		ImGui::TableSetupColumn("deployRuntimePlacementId");
		ImGui::TableSetupColumn("initialState");
		ImGui::TableSetupColumn("enabled");
		ImGui::TableHeadersRow();
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			m_WorldGameplayDocument.Get_Placements())
		{
			if (WORLD_PLACEMENT_KIND::DESTROYABLE != placement.eKind)
				continue;
			++destroyableCount;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(placement.placementId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%llu",
				static_cast<unsigned long long>(
					placement.deployRuntimePlacementId));
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::DestroyableState_ToString(
					placement.eInitialState));
			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(placement.isEnabled ? "true" : "false");
		}
		ImGui::EndTable();
	}
	ImGui::Text("destroyable rows: %zu", destroyableCount);
	ImGui::TextWrapped(
		"The Client document parser accepts this kind, but "
		"Publish-WorldGameplay.ps1 rejects every kind outside "
		"playerSpawn/npc/boss/triggerBox/collisionBox. G5 opens that gate.");
}

void Client::CMapTool::Render_DestructionNavigationRegions()
{
	if (!ImGui::CollapsingHeader("Navigation blocker regions"))
		return;

	ImGui::Text("Document: %s | regions: %zu",
		m_RuntimeBlockerDocument.Is_Ready() ? "READY" : "NOT READY",
		m_RuntimeBlockerDocument.Get_RegionCount());
	if (0 == m_RuntimeBlockerDocument.Get_RegionCount())
	{
		ImGui::TextWrapped(
			"No region is authored yet. The baked grid treats every wall "
			"footprint as walkable, so an INTACT wall has to add a blocker "
			"rather than remove one.");
		return;
	}

	if (!ImGui::BeginTable("DestructionNavRegions", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		return;
	}
	ImGui::TableSetupColumn("regionId");
	ImGui::TableSetupColumn("conditionId");
	ImGui::TableSetupColumn("activateWhenTrue");
	ImGui::TableSetupColumn("cells");
	ImGui::TableHeadersRow();
	for (size_t index = 0;
		index < m_RuntimeBlockerDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_RuntimeBlockerDocument.Get_Region(index);
		if (nullptr == region)
			continue;
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(region->id.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(region->conditionId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::TextUnformatted(
			region->activateWhenConditionTrue ? "true" : "false");
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%u",
			m_RuntimeBlockerDocument.Get_RegionCellCount(index));
	}
	ImGui::EndTable();
}

void Client::CMapTool::Render_DestructionDiagnostics()
{
	if (!ImGui::CollapsingHeader("Diagnostics"))
		return;

	ImGui::TextWrapped(
		"Measured on 2026-08-07. These are locked notes, not runtime state.");
	ImGui::BulletText(
		"Server and Shared contain no destroyable state, message or receiver.");
	ImGui::BulletText(
		"CDeployPropRuntime::Set_State has no product caller; only the dead "
		"Set_DeployPhase calls Set_State_All.");
	ImGui::BulletText(
		"All 85 wall footprints are walkable in the published navgrid, so "
		"navigation must block while INTACT and open while FRACTURED.");
	ImGui::BulletText(
		"Arena floor collapse is the opposite polarity of a wall and must not "
		"share one boolean.");
	ImGui::BulletText(
		"CServerNavigation exposes only Load/Find_Path/Project_Point and "
		"cannot mutate the grid after load.");
	ImGui::BulletText(
		"CPlayerSkillSystem keeps the raw X/Z after Project_Point, so skill "
		"movement passes through any blocker until G10.");
	ImGui::BulletText(
		"DEPLOY_ITR_02326 has no fractured mesh and its ao_off clip holds "
		"still for 1.833s then scales to zero at 1.900s.");
}

void Client::CMapTool::Render_DestructionAreaControls()
{
	const NAV_RUNTIME_BLOCKER_REGION* selectedRegion =
		m_RuntimeBlockerDocument.Get_Region(
			m_iSelectedRuntimeRegion);
	const char* preview =
		nullptr != selectedRegion ?
		selectedRegion->id.c_str() :
		"<none>";

	ImGui::SetNextItemWidth(320.f);
	if (ImGui::BeginCombo("Region", preview))
	{
		for (size_t index = 0;
			index < m_RuntimeBlockerDocument.Get_RegionCount();
			++index)
		{
			const NAV_RUNTIME_BLOCKER_REGION* region =
				m_RuntimeBlockerDocument.Get_Region(index);
			if (nullptr == region)
				continue;

			const bool_t selected =
				index == m_iSelectedRuntimeRegion;
			if (ImGui::Selectable(
				region->id.c_str(),
				selected))
			{
				m_iSelectedRuntimeRegion = index;
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("New"))
		ImGui::OpenPopup("New Destruction Area");

	if (ImGui::BeginPopupModal(
		"New Destruction Area",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText(
			"Name",
			m_RuntimeBlockerId,
			sizeof(m_RuntimeBlockerId));

		if (ImGui::RadioButton(
			"Block after destruction",
			m_RuntimeActivateWhenConditionTrue))
		{
			m_RuntimeActivateWhenConditionTrue = true;
		}
		if (ImGui::RadioButton(
			"Open after destruction",
			!m_RuntimeActivateWhenConditionTrue))
		{
			m_RuntimeActivateWhenConditionTrue = false;
		}

		if (ImGui::CollapsingHeader("Advanced"))
		{
			ImGui::InputText(
				"Condition",
				m_RuntimeConditionId,
				sizeof(m_RuntimeConditionId));
		}

		const bool_t canCreate =
			'\0' != m_RuntimeBlockerId[0] &&
			'\0' != m_RuntimeConditionId[0];
		ImGui::BeginDisabled(!canCreate);
		if (ImGui::Button("Create") &&
			m_RuntimeBlockerDocument.Add_Region(
				m_RuntimeBlockerId,
				m_RuntimeConditionId,
				m_RuntimeActivateWhenConditionTrue,
				m_NavigationStatus))
		{
			m_iSelectedRuntimeRegion =
				m_RuntimeBlockerDocument.Get_RegionCount() - 1;
			m_NavigationStatus = "Unsaved";
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	selectedRegion = m_RuntimeBlockerDocument.Get_Region(
		m_iSelectedRuntimeRegion);
	if (nullptr != selectedRegion &&
		ImGui::CollapsingHeader("Test"))
	{
		bool_t conditionValue =
			m_NavigationConditions[selectedRegion->conditionId];
		if (ImGui::Checkbox(
			"Destroyed",
			&conditionValue))
		{
			if (!Set_NavigationCondition(
				selectedRegion->conditionId,
				conditionValue))
			{
				m_NavigationStatus =
					"Re-enter ASSET_TEST before testing this region.";
			}
		}
	}
}
