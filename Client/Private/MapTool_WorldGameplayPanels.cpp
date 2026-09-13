#include "imgui.h"
#include "MapTool_Internal.h"
#include "Npc.h"
#include "WorldSequenceToolPanel.h"
#include "ActorCatalog.h"
#include "Model.h"
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




void Client::CMapTool::Render_WorldGameplayPanel(bool_t isAssetTest)
{
	ImGui::TextUnformatted("World Gameplay Authoring");
	if (ImGui::CollapsingHeader("Card Maze Setup / Help"))
	{
		ImGui::TextWrapped("%s", "카드미로: 자기 문양은 망치 한 방에 1스택, 3스택이면 개인 출구가 활성화됩니다.");
		ImGui::TextWrapped("%s", "cardmaze.return: 비활성 상태를 유지하고 movePlayer의 Target Position을 편집하면 전원 완료 후 복귀 위치가 바뀝니다. 기본값은 2관문입니다.");
		ImGui::TextWrapped("%s", "Camera 탭: cardmaze.follow는 기본 시점, cardmaze.telescope는 망원경 전체보기입니다.");
		ImGui::TextWrapped("%s", "출구 효과는 미선정입니다. Effect Tool에서 cardmaze.exit.heart / spade / club / diamond 그룹을 등록·배포하면 연결됩니다. 현재 출구 좌표는 HUD에도 표시됩니다.");
		ImGui::TextWrapped("%s", "행진 위치·시간을 편집한 뒤 Map 및 World Gameplay를 함께 publish하고 Server/Client를 재시작하세요. 네비 재베이크는 필요하지 않습니다.");
	}
	ImGui::TextDisabled(
		"Player spawn, Trigger Box actions, and Collision Boxes are Server authority.");
	ImGui::TextDisabled(
		"Options: Player Spawn, NPC, Boss, Trigger Box, Collision Box.");
	ImGui::TextWrapped("%s", m_WorldGameplayStatus.c_str());
	ImGui::Separator();
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	if (nullptr == active ||
		EDITOR_GAMEPLAY_POLICY::REQUIRED != active->gameplayPolicy)
	{
		ImGui::TextDisabled(
			"This Area has no gameplay authoring document. No empty file will be created.");
		return;
	}

	ImGui::BeginDisabled(!isAssetTest || !m_Catalog.Is_Ready());
	if (ImGui::Button("Save Gameplay"))
	{
		if (!m_WorldNpcBatchDraft.empty())
			m_WorldGameplayStatus =
				"Confirm or Discard the staged NPC batch before saving";
		else
			Save_WorldGameplay();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Gameplay"))
	{
		if (!m_WorldNpcBatchDraft.empty())
			m_WorldGameplayStatus =
				"Confirm or Discard the staged NPC batch before reloading";
		else if (m_bWorldNpcBehaviorDraftDirty)
			m_WorldGameplayStatus =
				"Apply or Revert the current NPC behavior draft before reloading";
		else
			Load_WorldGameplay();
	}
	ImGui::SameLine();
	ImGui::Text("Revision: %u | Placements: %zu%s",
		m_WorldGameplayDocument.Get_Revision(),
		m_WorldGameplayDocument.Get_Placements().size(),
		m_bWorldGameplayDirty ? "  *unsaved" : "");
	ImGui::Separator();

	if (ImGui::RadioButton("Player Spawn",
		WORLD_PLACEMENT_KIND::PLAYER_SPAWN == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::PLAYER_SPAWN;
	ImGui::SameLine();
	if (ImGui::RadioButton("NPC",
		WORLD_PLACEMENT_KIND::NPC == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::NPC;
	ImGui::SameLine();
	if (ImGui::RadioButton("Boss",
		WORLD_PLACEMENT_KIND::BOSS == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::BOSS;
	ImGui::SameLine();
	if (ImGui::RadioButton("Trigger Box",
		WORLD_PLACEMENT_KIND::TRIGGER_BOX == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::TRIGGER_BOX;
	ImGui::SameLine();
	if (ImGui::RadioButton("Collision Box",
		WORLD_PLACEMENT_KIND::COLLISION_BOX == m_eWorldPlacementKind))
		m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::COLLISION_BOX;

	ImGui::InputText("Placement ID", m_WorldPlacementId,
		std::size(m_WorldPlacementId));
	const bool_t isTriggerPlacement =
		WORLD_PLACEMENT_KIND::TRIGGER_BOX == m_eWorldPlacementKind;
	const bool_t isCollisionPlacement =
		WORLD_PLACEMENT_KIND::COLLISION_BOX == m_eWorldPlacementKind;
	if (isTriggerPlacement || isCollisionPlacement)
	{
		ImGui::DragFloat3(
			"Default Box Scale (Half Extents)",
			&m_WorldTriggerHalfExtents.x,
			0.1f,
			0.1f,
			1000.f,
			"%.2f",
			ImGuiSliderFlags_AlwaysClamp);
		if (isTriggerPlacement)
		{
			ImGui::Checkbox("Trigger Once", &m_bWorldTriggerOnce);
			ImGui::TextDisabled(
				"Place the box first, then select it and choose one typed action below.");
		}
		else
		{
			ImGui::TextDisabled(
				"Collision Box blocks Server-authoritative player walking.");
		}
	}
	else
	{
		if (WORLD_PLACEMENT_KIND::PLAYER_SPAWN == m_eWorldPlacementKind)
		{
			ImGui::TextDisabled(
				"Player Spawn owns position/yaw only; class is selected by the session.");
		}
		else if (WORLD_PLACEMENT_KIND::NPC == m_eWorldPlacementKind)
		{
			const std::vector<NPC_ACTOR_ENTRY>& npcs =
				CActorCatalog::Get_Npcs();
			if (npcs.empty())
			{
				m_WorldArchetypeId[0] = '\0';
				ImGui::TextDisabled(
					"NpcCatalog has no supported archetype.");
			}
			else
			{
				if (m_iWorldNpcArchetypeIndex < 0 ||
					m_iWorldNpcArchetypeIndex >=
						static_cast<int32_t>(npcs.size()))
				{
					m_iWorldNpcArchetypeIndex = 0;
				}
				if (ImGui::BeginCombo("NPC Archetype",
					npcs[m_iWorldNpcArchetypeIndex].archetypeId.c_str()))
				{
					for (int32_t index = 0;
						index < static_cast<int32_t>(npcs.size()); ++index)
					{
						const bool_t isSelected =
							index == m_iWorldNpcArchetypeIndex;
						if (ImGui::Selectable(
							npcs[index].archetypeId.c_str(), isSelected))
						{
							m_iWorldNpcArchetypeIndex = index;
							if (!isSelected)
								m_WorldNpcBrushPreset.reset();
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				strcpy_s(m_WorldArchetypeId,
					npcs[m_iWorldNpcArchetypeIndex].archetypeId.c_str());
				const std::string selectedAutoId = "npc." +
					Editor_AreaShortName(active->areaId) + "." +
					Editor_NpcArchetypeToken(
						npcs[m_iWorldNpcArchetypeIndex].archetypeId);
				const std::string currentId = m_WorldPlacementId;
				bool_t isAutoValue = "player.spawn.editor" == currentId;
				for (size_t index = 0;
					!isAutoValue && index < npcs.size(); ++index)
				{
					const std::string name =
						Editor_NpcArchetypeToken(npcs[index].archetypeId);
					isAutoValue = 0 == currentId.rfind("npc.", 0) &&
						currentId.size() > name.size() &&
						0 == currentId.compare(
							currentId.size() - name.size(),
							name.size(), name);
				}
				if (isAutoValue && selectedAutoId != currentId)
					strcpy_s(m_WorldPlacementId, selectedAutoId.c_str());
				ImGui::TextDisabled(
					"Archetypes come from Data/Actors/NpcCatalog.json (runtimeStatus=supported).");

				ImGui::SeparatorText("Quick NPC Brush");
				ImGui::TextDisabled(
					"Start once, then every map click creates another NPC with an automatic ID.");
				const WORLD_GAMEPLAY_PLACEMENT* brushSource =
					m_WorldGameplayDocument.Find(m_SelectedWorldPlacementId);
				const bool_t hasBrushSource = nullptr != brushSource &&
					WORLD_PLACEMENT_KIND::NPC == brushSource->eKind;
				ImGui::BeginDisabled(!hasBrushSource ||
					m_bWorldNpcBehaviorDraftDirty);
				if (ImGui::Button("Capture Selected NPC As Brush Preset"))
				{
					m_WorldNpcBrushPreset = *brushSource;
					const auto archetype = std::find_if(npcs.begin(), npcs.end(),
						[brushSource](const NPC_ACTOR_ENTRY& entry)
						{
							return entry.archetypeId == brushSource->archetypeId;
						});
					if (archetype != npcs.end())
						m_iWorldNpcArchetypeIndex = static_cast<int32_t>(
							std::distance(npcs.begin(), archetype));
					m_WorldGameplayStatus =
						"Quick NPC Brush preset captured from: " +
						brushSource->placementId;
				}
				ImGui::EndDisabled();
				if (m_WorldNpcBrushPreset.has_value())
				{
					ImGui::Text("Preset: %s / %s",
						m_WorldNpcBrushPreset->placementId.c_str(),
						m_WorldNpcBrushPreset->npcBehavior.has_value() ?
						CWorldGameplayDocument::NpcBehaviorMode_ToString(
							m_WorldNpcBrushPreset->npcBehavior->eMode) :
						"Idle Loop");
					ImGui::SameLine();
					if (ImGui::SmallButton("Clear Brush Preset"))
						m_WorldNpcBrushPreset.reset();
				}
				else
				{
					ImGui::TextDisabled(
						"No preset: the selected archetype uses its catalog idle loop.");
				}
				ImGui::Checkbox("Brush Random Yaw", &m_bWorldNpcBrushRandomYaw);
				const bool_t brushArmed = m_bWorldGameplayPlacementArmed &&
					m_bWorldNpcContinuousPlacement &&
					WORLD_PLACEMENT_KIND::NPC == m_eWorldPlacementKind;
				ImGui::BeginDisabled(!brushArmed &&
					(m_bWorldNpcBehaviorDraftDirty ||
					 !m_WorldNpcBatchDraft.empty()));
				if (ImGui::Button(brushArmed ?
					"Stop Continuous NPC Brush" :
					"Start Continuous NPC Brush"))
				{
					if (brushArmed)
					{
						m_bWorldGameplayPlacementArmed = false;
						m_bWorldNpcContinuousPlacement = false;
						m_WorldGameplayStatus = "Quick NPC Brush stopped";
					}
					else
					{
						m_eWorldPlacementKind = WORLD_PLACEMENT_KIND::NPC;
						m_bWorldGameplayPlacementArmed = true;
						m_bWorldNpcContinuousPlacement = true;
						m_bWorldNpcWaypointPickArmed = false;
						m_bWorldNpcBatchCenterPickArmed = false;
						m_bWorldTriggerTargetPickArmed = false;
						m_bSpawnAnchorPlacementArmed = false;
						m_WorldGameplayStatus =
							"Quick NPC Brush armed: click map surfaces repeatedly; Esc stops";
					}
				}
				ImGui::EndDisabled();
				if (brushArmed)
				{
					ImGui::TextColored(ImVec4(0.3f, 1.f, 0.45f, 1.f),
						"BRUSH ACTIVE: each world click commits one NPC; Save once when finished");
				}

				if (ImGui::CollapsingHeader("NPC Batch Placement"))
				{
					const auto clearBatchDraft = [this]()
					{
						m_WorldNpcBatchDraft.clear();
						m_iWorldNpcBatchDraftBaseRevision = 0;
					};
					ImGui::TextDisabled(
						"Stage deterministic ghost markers, then confirm one atomic transaction.");
					if (ImGui::Button("Add Selected Archetype To Pool"))
					{
						if (m_WorldNpcBatchArchetypePool.insert(
							npcs[m_iWorldNpcArchetypeIndex].archetypeId).second)
							clearBatchDraft();
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear Archetype Pool"))
					{
						m_WorldNpcBatchArchetypePool.clear();
						clearBatchDraft();
					}
					if (m_WorldNpcBatchArchetypePool.empty())
						ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
							"Add at least one NPC archetype to the batch pool.");
					else
					{
						std::vector<std::string> pool(
							m_WorldNpcBatchArchetypePool.begin(),
							m_WorldNpcBatchArchetypePool.end());
						std::sort(pool.begin(), pool.end());
						for (const std::string& archetypeId : pool)
						{
							ImGui::PushID(archetypeId.c_str());
							ImGui::BulletText("%s", archetypeId.c_str());
							ImGui::SameLine();
							if (ImGui::SmallButton("Remove"))
							{
								m_WorldNpcBatchArchetypePool.erase(archetypeId);
								clearBatchDraft();
							}
							ImGui::PopID();
						}
					}
					if (ImGui::InputText("Batch ID Prefix", m_WorldNpcBatchIdPrefix,
						std::size(m_WorldNpcBatchIdPrefix)))
						clearBatchDraft();
					int32_t batchCount = static_cast<int32_t>(m_iWorldNpcBatchCount);
					if (ImGui::DragInt("Batch Count", &batchCount, 1.f, 1, 128,
						"%d", ImGuiSliderFlags_AlwaysClamp))
					{
						m_iWorldNpcBatchCount = static_cast<uint32_t>(batchCount);
						clearBatchDraft();
					}
					if (ImGui::DragFloat("Batch Radius", &m_fWorldNpcBatchRadius,
						0.25f, 0.5f, 100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
						clearBatchDraft();
					if (ImGui::DragFloat("Minimum Spacing", &m_fWorldNpcBatchMinimumSpacing,
						0.1f, 0.25f, 20.f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
						clearBatchDraft();
					int32_t batchSeed = static_cast<int32_t>(
						(std::min)(m_iWorldNpcBatchSeed, static_cast<uint32_t>(INT32_MAX)));
					if (ImGui::DragInt("Batch Seed", &batchSeed, 1.f, 1, INT32_MAX,
						"%d", ImGuiSliderFlags_AlwaysClamp))
					{
						m_iWorldNpcBatchSeed = static_cast<uint32_t>(batchSeed);
						clearBatchDraft();
					}
					if (ImGui::Checkbox("Random Yaw", &m_bWorldNpcBatchRandomYaw))
						clearBatchDraft();
					const WORLD_GAMEPLAY_PLACEMENT* behaviorSource =
						m_WorldGameplayDocument.Find(m_SelectedWorldPlacementId);
					const bool_t hasBehaviorSource = nullptr != behaviorSource &&
						WORLD_PLACEMENT_KIND::NPC == behaviorSource->eKind &&
						behaviorSource->npcBehavior.has_value();
					ImGui::BeginDisabled(!hasBehaviorSource);
					if (ImGui::Checkbox("Copy Applied Behavior From Selected NPC",
						&m_bWorldNpcBatchCopySelectedBehavior))
						clearBatchDraft();
					ImGui::EndDisabled();
					if (!hasBehaviorSource && m_bWorldNpcBatchCopySelectedBehavior)
					{
						m_bWorldNpcBatchCopySelectedBehavior = false;
						clearBatchDraft();
					}
					if (ImGui::Button(m_bWorldNpcBatchCenterPickArmed ?
						"Cancel Batch Center Pick" : "Pick Batch Center On Map"))
					{
						m_bWorldNpcBatchCenterPickArmed =
							!m_bWorldNpcBatchCenterPickArmed;
						m_bWorldGameplayPlacementArmed = false;
						m_bWorldNpcWaypointPickArmed = false;
						m_bWorldTriggerTargetPickArmed = false;
						m_bSpawnAnchorPlacementArmed = false;
						m_WorldGameplayStatus = m_bWorldNpcBatchCenterPickArmed ?
							"NPC batch center armed: click a map surface" :
							"NPC batch center pick cancelled";
					}
					if (m_bWorldNpcBatchCenterValid)
						ImGui::Text("Center: %.2f, %.2f, %.2f",
							m_WorldNpcBatchCenter.x, m_WorldNpcBatchCenter.y,
							m_WorldNpcBatchCenter.z);
					ImGui::BeginDisabled(!m_bWorldNpcBatchCenterValid ||
						m_WorldNpcBatchArchetypePool.empty() ||
						m_bWorldNpcBehaviorDraftDirty);
					if (ImGui::Button("Stage NPC Batch Ghost"))
						Place_WorldNpcBatch();
					ImGui::EndDisabled();
					if (!m_WorldNpcBatchDraft.empty())
					{
						const bool_t revisionMatches =
							m_iWorldNpcBatchDraftBaseRevision ==
								m_WorldGameplayDocument.Get_Revision();
						ImGui::Text("Staged ghost placements: %zu%s",
							m_WorldNpcBatchDraft.size(),
							revisionMatches ? "" : " (document changed; re-stage required)");
						ImGui::BeginDisabled(!revisionMatches);
						if (ImGui::Button("Confirm NPC Batch"))
							Commit_WorldNpcBatch();
						ImGui::EndDisabled();
						ImGui::SameLine();
						if (ImGui::Button("Discard NPC Batch Ghost"))
							clearBatchDraft();
					}
				}
			}
			m_WorldEncounterId[0] = '\0';
		}
		else
		{
			ImGui::InputText("Archetype ID", m_WorldArchetypeId,
				std::size(m_WorldArchetypeId));
			ImGui::InputText("Encounter ID (optional)", m_WorldEncounterId,
				std::size(m_WorldEncounterId));
		}
	}
	if (ImGui::Button(m_bWorldGameplayPlacementArmed ?
		"Cancel World Placement" : "Arm World Placement"))
	{
		m_bWorldNpcContinuousPlacement = false;
		m_bWorldGameplayPlacementArmed =
			!m_bWorldGameplayPlacementArmed;
		if (m_bWorldGameplayPlacementArmed)
		{
			m_bWorldNpcWaypointPickArmed = false;
			m_bWorldNpcBatchCenterPickArmed = false;
			m_bWorldTriggerTargetPickArmed = false;
			m_bSpawnAnchorPlacementArmed = false;
		}
		m_WorldGameplayStatus = m_bWorldGameplayPlacementArmed ?
			"World placement armed: click a picked map surface; Esc cancels" :
			"World placement cancelled";
	}
	if (m_bWorldGameplayPlacementArmed)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PICKING: click the map surface to store the placement position");
	}

	ImGui::Separator();
	if (ImGui::BeginTable("WorldGameplayPlacements", 5,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY, ImVec2(0.f, 280.f)))
	{
		ImGui::TableSetupColumn("Placement ID");
		ImGui::TableSetupColumn("Kind");
		ImGui::TableSetupColumn("Archetype");
		ImGui::TableSetupColumn("Position");
		ImGui::TableSetupColumn("Enabled");
		ImGui::TableHeadersRow();
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			m_WorldGameplayDocument.Get_Placements())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t selected =
				m_SelectedWorldPlacementId == placement.placementId;
			if (ImGui::Selectable(placement.placementId.c_str(), selected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				if (!selected && !m_WorldNpcBatchDraft.empty())
				{
					m_WorldGameplayStatus =
						"Confirm or Discard the staged NPC batch before changing selection";
				}
				else if (!selected && m_bWorldNpcBehaviorDraftDirty)
				{
					m_WorldGameplayStatus =
						"Apply or Revert the current NPC behavior draft before changing selection";
				}
				else
				{
					m_SelectedWorldPlacementId = placement.placementId;
					m_WorldPlacementPositionDelta = {};
				}
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::Kind_ToString(placement.eKind));
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(placement.archetypeId.c_str());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%.2f, %.2f, %.2f", placement.position.x,
				placement.position.y, placement.position.z);
			ImGui::TableSetColumnIndex(4);
			ImGui::TextUnformatted(placement.isEnabled ? "yes" : "no");
		}
		ImGui::EndTable();
	}

	WORLD_GAMEPLAY_PLACEMENT* selected =
		m_WorldGameplayDocument.Find(m_SelectedWorldPlacementId);
	if (nullptr != selected)
	{
		ImGui::SeparatorText("Selected Gameplay Placement");
		WORLD_GAMEPLAY_PLACEMENT staged = *selected;
		bool_t edited = false;
		bool_t quickNpcPresetEdited = false;
		edited |= ImGui::DragFloat3("Position", &staged.position.x,
			0.1f, -100000.f, 100000.f, "%.3f");
		edited |= ImGui::DragFloat("Yaw Degrees", &staged.yawDegrees,
			0.5f, -360.f, 360.f, "%.2f");
		if (WORLD_PLACEMENT_KIND::PLAYER_SPAWN == staged.eKind)
		{
			ImGui::SeparatorText("Player Spawn Position Offset");
			ImGui::DragFloat3(
				"Position Delta",
				&m_WorldPlacementPositionDelta.x,
				0.1f,
				-100000.f,
				100000.f,
				"%+.3f");
			if (ImGui::Button("Apply Delta To Spawn Position"))
			{
				staged.position.x += m_WorldPlacementPositionDelta.x;
				staged.position.y += m_WorldPlacementPositionDelta.y;
				staged.position.z += m_WorldPlacementPositionDelta.z;
				m_WorldPlacementPositionDelta = {};
				edited = true;
			}
			ImGui::TextDisabled(
				"Example: +50, 0, 0 resolves and saves Position.x + 50; Server uses the saved result.");
		}
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX == staged.eKind)
		{
			edited |= ImGui::DragFloat3(
				"Half Extents",
				&staged.halfExtents.x,
				0.1f,
				0.1f,
				1000.f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);
			edited |= ImGui::Checkbox("Trigger Once", &staged.isTriggerOnce);

			int actionOption = 0;
			if (1u == staged.triggerEvents.size())
			{
				switch (staged.triggerEvents.front().eKind)
				{
				case WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER:
					actionOption = 1;
					break;
				case WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL:
					actionOption = 2;
					break;
				case WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP:
					actionOption = 3;
					break;
				case WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER:
					actionOption = 4;
					break;
				case WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE:
					actionOption = 5;
					break;
				case WORLD_TRIGGER_EVENT_KIND::CLAIM_CARD_MAZE_TELESCOPE:
					actionOption = 6;
					break;
				default:
					break;
				}
			}
			const char_t* actionOptions[] =
			{
				"None", "Move Player", "Change Level",
				"Activate Spawn Group", "Activate Encounter", "Play Sequence",
				"Claim Card Maze Telescope"
			};
			if (ImGui::Combo("Action", &actionOption,
				actionOptions, static_cast<int>(std::size(actionOptions))))
			{
				m_bWorldTriggerTargetPickArmed = false;
				if (0 == actionOption)
				{
					staged.triggerEvents.clear();
					staged.isEnabled = false;
				}
				else
				{
					WORLD_TRIGGER_EVENT action{};
					if (1 == actionOption)
					{
						action.eKind = WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER;
						action.targetPosition = staged.position;
					}
					else if (2 == actionOption)
					{
						action.eKind = WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL;
						action.eTargetWorldId =
							LostArk::Shared::WORLD_ID::VALTAN_ARENA;
					}
					else if (5 == actionOption)
					{
						/* Seed the first authored instance so the field is
						   never blank; the author picks the real one below. */
						action.eKind = WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE;
						const std::vector<std::string> instanceIds =
							nullptr == m_pWorldSequenceToolPanel ?
								std::vector<std::string>{} :
								m_pWorldSequenceToolPanel->Get_InstanceIds();
						if (!instanceIds.empty())
							action.targetId = instanceIds.front();
					}
					else if (6 == actionOption)
					{
						/* The claim is a stable id of its own; the box itself is
						   the natural name. The Server measures the hammer swing
						   against this box, so it is not interact-gated. */
						action.eKind = WORLD_TRIGGER_EVENT_KIND::CLAIM_CARD_MAZE_TELESCOPE;
						action.targetId = staged.placementId;
					}
					else
					{
						action.eKind = 3 == actionOption ?
							WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP :
							WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER;
						if (3 == actionOption &&
							!m_SpawnGroupDocument.Get_Groups().empty())
						{
							action.targetId =
								m_SpawnGroupDocument.Get_Groups().front().spawnGroupId;
						}
						else if (4 == actionOption)
						{
							const auto boss = std::find_if(
								m_WorldGameplayDocument.Get_Placements().begin(),
								m_WorldGameplayDocument.Get_Placements().end(),
								[](const WORLD_GAMEPLAY_PLACEMENT& value)
								{
									return WORLD_PLACEMENT_KIND::BOSS == value.eKind;
								});
							if (m_WorldGameplayDocument.Get_Placements().end() != boss)
								action.targetId = boss->placementId;
						}
					}
					staged.triggerEvents.assign(1u, action);
				}
				edited = true;
			}
			const bool_t hasMoveAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER ==
					staged.triggerEvents.front().eKind;
			if (hasMoveAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Move Player Action");
				edited |= ImGui::DragFloat3(
					"Target Position", &action.targetPosition.x,
					0.1f, -100000.f, 100000.f, "%.3f");
				edited |= ImGui::DragFloat(
					"Duration Seconds", &action.durationSeconds,
					0.05f, 0.05f, 10.f, "%.2f",
					ImGuiSliderFlags_AlwaysClamp);
				edited |= ImGui::DragFloat(
					"Arc Height", &action.arcHeight,
					0.1f, 0.f, 1000.f, "%.2f",
					ImGuiSliderFlags_AlwaysClamp);
				if (ImGui::Button(m_bWorldTriggerTargetPickArmed ?
					"Cancel Move Target Pick" : "Pick Move Target On Map"))
				{
					m_bWorldTriggerTargetPickArmed =
						!m_bWorldTriggerTargetPickArmed;
					m_bWorldGameplayPlacementArmed = false;
					m_WorldGameplayStatus = m_bWorldTriggerTargetPickArmed ?
						"Move target armed: click the destination surface; Esc cancels" :
						"Move target pick cancelled";
				}
			}
			const bool_t hasChangeLevelAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::CHANGE_LEVEL ==
					staged.triggerEvents.front().eKind;
			if (hasChangeLevelAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Change Level Action");
				int targetOption =
					LostArk::Shared::WORLD_ID::BERN == action.eTargetWorldId ? 0 : 1;
				const char_t* targetOptions[] = { "BERN", "VALTAN_ARENA" };
				if (ImGui::Combo("Target World", &targetOption,
					targetOptions, static_cast<int>(std::size(targetOptions))))
				{
					action.eTargetWorldId = 0 == targetOption ?
						LostArk::Shared::WORLD_ID::BERN :
						LostArk::Shared::WORLD_ID::VALTAN_ARENA;
					edited = true;
				}
				ImGui::TextDisabled(
					"The Server changes rooms first; Client consumes S2C_ENTER_ACCEPTED.");
			}
			const bool_t hasSpawnGroupAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP ==
					staged.triggerEvents.front().eKind;
			if (hasSpawnGroupAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Activate Spawn Group Action");
				const char_t* preview = action.targetId.empty() ?
					"<select spawn group>" : action.targetId.c_str();
				if (ImGui::BeginCombo("Spawn Group", preview))
				{
					for (const SPAWN_GROUP_RECORD& group :
						m_SpawnGroupDocument.Get_Groups())
					{
						const bool_t isSelected = action.targetId == group.spawnGroupId;
						if (ImGui::Selectable(group.spawnGroupId.c_str(), isSelected))
						{
							action.targetId = group.spawnGroupId;
							edited = true;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::TextDisabled(
					"Server activates this stable group; prerequisite and once rules stay authoritative.");
			}
			const bool_t hasEncounterAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER ==
					staged.triggerEvents.front().eKind;
			if (hasEncounterAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Activate Encounter Action");
				const char_t* preview = action.targetId.empty() ?
					"<select disabled boss placement>" : action.targetId.c_str();
				if (ImGui::BeginCombo("Boss Placement", preview))
				{
					for (const WORLD_GAMEPLAY_PLACEMENT& placement :
						m_WorldGameplayDocument.Get_Placements())
					{
						if (WORLD_PLACEMENT_KIND::BOSS != placement.eKind)
							continue;
						const bool_t isSelected = action.targetId == placement.placementId;
						if (ImGui::Selectable(placement.placementId.c_str(), isSelected))
						{
							action.targetId = placement.placementId;
							edited = true;
						}
					}
					ImGui::EndCombo();
				}
			}

			const bool_t hasTelescopeAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::CLAIM_CARD_MAZE_TELESCOPE ==
					staged.triggerEvents.front().eKind;
			if (hasTelescopeAction)
			{
				ImGui::SeparatorText("Claim Card Maze Telescope Action");
				ImGui::TextDisabled("Target id: %s",
					staged.triggerEvents.front().targetId.c_str());
				ImGui::TextWrapped(
					"The first player whose hammer swing reaches this box becomes "
					"the telescope owner; the Server then deals the suits and raises "
					"the targets. Walking in does nothing. Only the Kouku arena accepts it.");
			}

			const bool_t hasPlaySequenceAction =
				1u == staged.triggerEvents.size() &&
				WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE ==
					staged.triggerEvents.front().eKind;
			if (hasPlaySequenceAction)
			{
				WORLD_TRIGGER_EVENT& action = staged.triggerEvents.front();
				ImGui::SeparatorText("Play Sequence Action");
				const char_t* preview = action.targetId.empty() ?
					"<select sequence instance>" : action.targetId.c_str();
				const std::vector<std::string> instanceIds =
					nullptr == m_pWorldSequenceToolPanel ?
						std::vector<std::string>{} :
						m_pWorldSequenceToolPanel->Get_InstanceIds();
				if (ImGui::BeginCombo("Sequence Instance", preview))
				{
					for (const std::string& instanceId : instanceIds)
					{
						const bool_t isSelected = action.targetId == instanceId;
						if (ImGui::Selectable(instanceId.c_str(), isSelected))
						{
							action.targetId = instanceId;
							edited = true;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::TextDisabled(
					"Plays one authored world sequence instance from this Area.");
			}

			const bool_t hasSupportedAction =
				hasMoveAction || hasChangeLevelAction ||
				hasSpawnGroupAction || hasEncounterAction ||
				hasPlaySequenceAction;
			ImGui::BeginDisabled(!hasSupportedAction);
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
			ImGui::EndDisabled();
			if (!hasSupportedAction)
			{
				staged.isEnabled = false;
				ImGui::TextDisabled(
					"A Trigger Box needs exactly one supported action before it can be enabled.");
			}
		}
		else if (WORLD_PLACEMENT_KIND::COLLISION_BOX == staged.eKind)
		{
			edited |= ImGui::DragFloat3(
				"Half Extents",
				&staged.halfExtents.x,
				0.1f,
				0.1f,
				1000.f,
				"%.2f",
				ImGuiSliderFlags_AlwaysClamp);
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
		}
		else if (WORLD_PLACEMENT_KIND::NPC == staged.eKind)
		{
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
			Sync_WorldNpcBehaviorDraft(staged);
			const auto preview = std::find_if(
				m_WorldNpcPreviews.begin(), m_WorldNpcPreviews.end(),
				[&](const NPC_PREVIEW_ENTRY& entry)
				{
					return entry.placementId == staged.placementId;
				});
			if (m_WorldNpcPreviews.end() != preview &&
				nullptr != preview->object &&
				nullptr != preview->object->Get_Model())
			{
				ImGui::SeparatorText("NPC Preview Animation");
				const shared_ptr<CModel> model =
					preview->object->Get_Model();
				const uint32_t clipCount = model->Get_NumAnimations();
				const char_t* currentClip =
					model->Get_AnimationName(model->Get_CurrentAnimIndex());
				if (ImGui::BeginCombo("Preview Clip",
					nullptr != currentClip ? currentClip : "<none>"))
				{
					for (uint32_t clipIndex = 0;
						clipIndex < clipCount; ++clipIndex)
					{
						const char_t* clipName =
							model->Get_AnimationName(clipIndex);
						if (nullptr == clipName)
							continue;
						const bool_t isSelected =
							clipIndex == model->Get_CurrentAnimIndex();
						if (ImGui::Selectable(clipName, isSelected))
							preview->object->Set_Animation(clipName, true);
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				if (ImGui::Button("Play / Restart Preview Clip"))
					model->Start_Animation(model->Get_CurrentAnimIndex(), true);
				ImGui::SameLine();
				if (ImGui::Button("Pause Preview Clip"))
					model->Stop_Animation();

				ImGui::SeparatorText("Easy NPC Setup");
				ImGui::TextDisabled(
					"Choose a real Preview Clip above, then apply one complete preset.");
				ImGui::DragFloat("Easy Wander Radius",
					&m_fWorldNpcQuickWanderRadius, 0.25f, 0.5f, 100.f,
					"%.2f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::DragFloat("Easy Move Speed",
					&m_fWorldNpcQuickMoveSpeed, 0.05f, 0.1f, 10.f,
					"%.2f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::BeginDisabled(nullptr == currentClip);
				if (ImGui::Button("1-Click: Stationary Idle Loop"))
				{
					staged.npcIdleClip = currentClip;
					staged.npcBehavior.reset();
					m_WorldNpcBehaviorDraftPlacementId = staged.placementId;
					m_WorldNpcBehaviorDraft.reset();
					m_bWorldNpcBehaviorDraftDirty = false;
					quickNpcPresetEdited = true;
					edited = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("1-Click: Wander With Preview Walk"))
				{
					WORLD_NPC_BEHAVIOR behavior;
					behavior.eMode = WORLD_NPC_BEHAVIOR_MODE::WANDER;
					behavior.eRouteMode = WORLD_NPC_ROUTE_MODE::LOOP;
					behavior.eActionSelection =
						WORLD_NPC_ACTION_SELECTION::SEQUENCE;
					behavior.walkClip = currentClip;
					behavior.moveSpeed = m_fWorldNpcQuickMoveSpeed;
					behavior.wanderRadius = m_fWorldNpcQuickWanderRadius;
					behavior.randomSeed = Editor_StableSeed(staged.placementId);
					behavior.startDelayMs = 0u;
					behavior.idleMinMs = 1000u;
					behavior.idleMaxMs = 3000u;
					staged.npcBehavior = behavior;
					m_WorldNpcBehaviorDraftPlacementId = staged.placementId;
					m_WorldNpcBehaviorDraft = behavior;
					m_bWorldNpcBehaviorDraftDirty = false;
					quickNpcPresetEdited = true;
					edited = true;
				}
				ImGui::EndDisabled();
				ImGui::TextDisabled(
					"Stationary clears movement. Wander uses the selected clip as its walk loop.");
				if (nullptr != currentClip &&
					ImGui::Button("Use As Placement Idle"))
				{
					staged.npcIdleClip = currentClip;
					edited = true;
				}
				if (!staged.npcIdleClip.empty())
				{
					ImGui::SameLine();
					if (ImGui::Button("Clear Placement Idle"))
					{
						staged.npcIdleClip.clear();
						edited = true;
					}
					ImGui::Text("Saved idle: %s",
						staged.npcIdleClip.c_str());
				}
				else
				{
					ImGui::TextDisabled(
						"No placement idle saved; the catalog idleClip plays.");
				}
			}

			ImGui::SeparatorText("NPC Behavior Authoring");
			bool_t behaviorEnabled = m_WorldNpcBehaviorDraft.has_value();
			if (ImGui::Checkbox("Enable Behavior", &behaviorEnabled))
			{
				if (behaviorEnabled)
					m_WorldNpcBehaviorDraft.emplace();
				else
					m_WorldNpcBehaviorDraft.reset();
				m_bWorldNpcBehaviorDraftDirty = true;
				m_bWorldNpcWaypointPickArmed = false;
			}
			if (m_WorldNpcBehaviorDraft.has_value())
			{
				WORLD_NPC_BEHAVIOR& behavior = *m_WorldNpcBehaviorDraft;
				int32_t mode = static_cast<int32_t>(behavior.eMode);
				const char_t* modeLabels[] = { "Stationary", "Patrol", "Wander" };
				if (ImGui::Combo("Behavior Mode", &mode, modeLabels,
					static_cast<int32_t>(std::size(modeLabels))))
				{
					behavior.eMode = static_cast<WORLD_NPC_BEHAVIOR_MODE>(mode);
					if (WORLD_NPC_BEHAVIOR_MODE::STATIONARY == behavior.eMode)
					{
						behavior.waypoints.clear();
						behavior.wanderRadius = 0.f;
					}
					else if (WORLD_NPC_BEHAVIOR_MODE::PATROL == behavior.eMode)
					{
						behavior.wanderRadius = 0.f;
					}
					else
					{
						behavior.waypoints.clear();
						behavior.wanderRadius = (std::max)(0.5f, behavior.wanderRadius);
					}
					m_bWorldNpcBehaviorDraftDirty = true;
				}
				int32_t routeMode = static_cast<int32_t>(behavior.eRouteMode);
				const char_t* routeLabels[] = { "Loop", "Ping Pong", "Once" };
				if (ImGui::Combo("Route Mode", &routeMode, routeLabels,
					static_cast<int32_t>(std::size(routeLabels))))
				{
					behavior.eRouteMode = static_cast<WORLD_NPC_ROUTE_MODE>(routeMode);
					m_bWorldNpcBehaviorDraftDirty = true;
				}
				int32_t actionSelection =
					static_cast<int32_t>(behavior.eActionSelection);
				const char_t* selectionLabels[] = { "Sequence", "Weighted" };
				if (ImGui::Combo("Action Selection", &actionSelection,
					selectionLabels, static_cast<int32_t>(std::size(selectionLabels))))
				{
					behavior.eActionSelection =
						static_cast<WORLD_NPC_ACTION_SELECTION>(actionSelection);
					m_bWorldNpcBehaviorDraftDirty = true;
				}
				m_bWorldNpcBehaviorDraftDirty |= ImGui::DragFloat(
					"Move Speed", &behavior.moveSpeed, 0.05f, 0.1f, 10.f,
					"%.2f", ImGuiSliderFlags_AlwaysClamp);
				if (WORLD_NPC_BEHAVIOR_MODE::WANDER == behavior.eMode)
				{
					m_bWorldNpcBehaviorDraftDirty |= ImGui::DragFloat(
						"Wander Radius", &behavior.wanderRadius, 0.25f, 0.5f,
						100.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
				}
				int32_t randomSeed = static_cast<int32_t>(
					(std::min)(behavior.randomSeed, static_cast<uint32_t>(INT32_MAX)));
				if (ImGui::DragInt("Random Seed", &randomSeed, 1.f, 1, INT32_MAX,
					"%d", ImGuiSliderFlags_AlwaysClamp))
				{
					behavior.randomSeed = static_cast<uint32_t>(randomSeed);
					m_bWorldNpcBehaviorDraftDirty = true;
				}
				int32_t startDelay = static_cast<int32_t>(behavior.startDelayMs);
				int32_t idleMinimum = static_cast<int32_t>(behavior.idleMinMs);
				int32_t idleMaximum = static_cast<int32_t>(behavior.idleMaxMs);
				if (ImGui::DragInt("Start Delay (ms)", &startDelay, 25.f, 0, 600000,
					"%d", ImGuiSliderFlags_AlwaysClamp))
				{
					behavior.startDelayMs = static_cast<uint32_t>(startDelay);
					m_bWorldNpcBehaviorDraftDirty = true;
				}
				if (ImGui::DragInt("Idle Minimum (ms)", &idleMinimum, 25.f, 0, 600000,
					"%d", ImGuiSliderFlags_AlwaysClamp))
				{
					behavior.idleMinMs = static_cast<uint32_t>(idleMinimum);
					m_bWorldNpcBehaviorDraftDirty = true;
				}
				if (ImGui::DragInt("Idle Maximum (ms)", &idleMaximum, 25.f, 0, 600000,
					"%d", ImGuiSliderFlags_AlwaysClamp))
				{
					behavior.idleMaxMs = static_cast<uint32_t>(idleMaximum);
					m_bWorldNpcBehaviorDraftDirty = true;
				}

				shared_ptr<CModel> behaviorModel;
				if (m_WorldNpcPreviews.end() != preview && nullptr != preview->object)
					behaviorModel = preview->object->Get_Model();
				if (nullptr != behaviorModel)
				{
					const char_t* walkPreview = behavior.walkClip.empty() ?
						"<select walk clip>" : behavior.walkClip.c_str();
					if (ImGui::BeginCombo("Walk Clip", walkPreview))
					{
						for (uint32_t clipIndex = 0;
							clipIndex < behaviorModel->Get_NumAnimations(); ++clipIndex)
						{
							const char_t* clipName = behaviorModel->Get_AnimationName(clipIndex);
							if (nullptr == clipName)
								continue;
							const bool_t isSelected = behavior.walkClip == clipName;
							if (ImGui::Selectable(clipName, isSelected))
							{
								behavior.walkClip = clipName;
								m_bWorldNpcBehaviorDraftDirty = true;
							}
						}
						ImGui::EndCombo();
					}
				}

				const char_t* lookTargetPreview = behavior.lookTargetPlacementId.empty() ?
					"<none>" : behavior.lookTargetPlacementId.c_str();
				if (ImGui::BeginCombo("Look Target NPC", lookTargetPreview))
				{
					if (ImGui::Selectable("<none>", behavior.lookTargetPlacementId.empty()))
					{
						behavior.lookTargetPlacementId.clear();
						m_bWorldNpcBehaviorDraftDirty = true;
					}
					for (const WORLD_GAMEPLAY_PLACEMENT& candidate :
						m_WorldGameplayDocument.Get_Placements())
					{
						if (WORLD_PLACEMENT_KIND::NPC != candidate.eKind ||
							!candidate.isEnabled ||
							candidate.placementId == staged.placementId)
							continue;
						const bool_t isSelected =
							behavior.lookTargetPlacementId == candidate.placementId;
						if (ImGui::Selectable(candidate.placementId.c_str(), isSelected))
						{
							behavior.lookTargetPlacementId = candidate.placementId;
							m_bWorldNpcBehaviorDraftDirty = true;
						}
					}
					ImGui::EndCombo();
				}

				if (WORLD_NPC_BEHAVIOR_MODE::PATROL == behavior.eMode)
				{
					ImGui::SeparatorText("Patrol Waypoints");
					for (size_t waypointIndex = 0;
						waypointIndex < behavior.waypoints.size();)
					{
						WORLD_NPC_WAYPOINT& waypoint = behavior.waypoints[waypointIndex];
						ImGui::PushID(static_cast<int32_t>(waypointIndex));
						char waypointId[129]{};
						strcpy_s(waypointId, waypoint.waypointId.c_str());
						if (ImGui::InputText("Waypoint ID", waypointId, std::size(waypointId)))
						{
							waypoint.waypointId = waypointId;
							m_bWorldNpcBehaviorDraftDirty = true;
						}
						m_bWorldNpcBehaviorDraftDirty |= ImGui::DragFloat3(
							"Waypoint Position", &waypoint.position.x, 0.1f,
							-100000.f, 100000.f, "%.3f");
						int32_t waitMs = static_cast<int32_t>(waypoint.waitMs);
						if (ImGui::DragInt("Wait (ms)", &waitMs, 25.f, 0, 600000,
							"%d", ImGuiSliderFlags_AlwaysClamp))
						{
							waypoint.waitMs = static_cast<uint32_t>(waitMs);
							m_bWorldNpcBehaviorDraftDirty = true;
						}
						bool_t hasLookYaw = waypoint.lookYawDegrees.has_value();
						if (ImGui::Checkbox("Use Arrival Yaw", &hasLookYaw))
						{
							if (hasLookYaw) waypoint.lookYawDegrees = 0.f;
							else waypoint.lookYawDegrees.reset();
							m_bWorldNpcBehaviorDraftDirty = true;
						}
						if (waypoint.lookYawDegrees.has_value())
							m_bWorldNpcBehaviorDraftDirty |= ImGui::DragFloat(
								"Arrival Yaw", &*waypoint.lookYawDegrees, 0.5f,
								-360.f, 360.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						bool_t removed = false;
						if (ImGui::SmallButton("Up") && 0u < waypointIndex)
						{
							std::swap(behavior.waypoints[waypointIndex - 1u],
								behavior.waypoints[waypointIndex]);
							m_bWorldNpcBehaviorDraftDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Down") &&
							waypointIndex + 1u < behavior.waypoints.size())
						{
							std::swap(behavior.waypoints[waypointIndex],
								behavior.waypoints[waypointIndex + 1u]);
							m_bWorldNpcBehaviorDraftDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Delete"))
						{
							behavior.waypoints.erase(behavior.waypoints.begin() + waypointIndex);
							m_bWorldNpcBehaviorDraftDirty = true;
							removed = true;
						}
						ImGui::Separator();
						ImGui::PopID();
						if (!removed) ++waypointIndex;
					}
					ImGui::BeginDisabled(behavior.waypoints.size() >= 64u);
					if (ImGui::Button(m_bWorldNpcWaypointPickArmed ?
						"Cancel Waypoint Pick" : "Add Waypoint From Map"))
					{
						m_bWorldNpcWaypointPickArmed = !m_bWorldNpcWaypointPickArmed;
						m_bWorldGameplayPlacementArmed = false;
						m_bWorldNpcBatchCenterPickArmed = false;
						m_bWorldTriggerTargetPickArmed = false;
						m_bSpawnAnchorPlacementArmed = false;
						m_WorldGameplayStatus = m_bWorldNpcWaypointPickArmed ?
							"NPC waypoint armed: click a map surface" :
							"NPC waypoint pick cancelled";
					}
					ImGui::EndDisabled();
					ImGui::TextDisabled("Route order: first row to last row; Loop/Ping Pong/Once controls the edge.");
				}

				ImGui::SeparatorText("Ambient Actions");
				for (size_t actionIndex = 0; actionIndex < behavior.actions.size();)
				{
					WORLD_NPC_ACTION& action = behavior.actions[actionIndex];
					ImGui::PushID(static_cast<int32_t>(1000u + actionIndex));
					char actionId[129]{};
					strcpy_s(actionId, action.actionId.c_str());
					if (ImGui::InputText("Action ID", actionId, std::size(actionId)))
					{
						action.actionId = actionId;
						m_bWorldNpcBehaviorDraftDirty = true;
					}
					if (nullptr != behaviorModel &&
						ImGui::BeginCombo("Action Clip", action.clipName.c_str()))
					{
						for (uint32_t clipIndex = 0;
							clipIndex < behaviorModel->Get_NumAnimations(); ++clipIndex)
						{
							const char_t* clipName = behaviorModel->Get_AnimationName(clipIndex);
							if (nullptr == clipName) continue;
							const bool_t isSelected = action.clipName == clipName;
							if (ImGui::Selectable(clipName, isSelected))
							{
								action.clipName = clipName;
								m_bWorldNpcBehaviorDraftDirty = true;
							}
						}
						ImGui::EndCombo();
					}
					m_bWorldNpcBehaviorDraftDirty |= ImGui::Checkbox("Loop", &action.loop);
					int32_t durationMs = static_cast<int32_t>(action.durationMs);
					int32_t waitAfterMs = static_cast<int32_t>(action.waitAfterMs);
					int32_t weight = static_cast<int32_t>(action.weight);
					if (ImGui::DragInt("Duration (ms)", &durationMs, 25.f, 1, 600000,
						"%d", ImGuiSliderFlags_AlwaysClamp))
					{
						action.durationMs = static_cast<uint32_t>(durationMs);
						m_bWorldNpcBehaviorDraftDirty = true;
					}
					if (ImGui::DragInt("Wait After (ms)", &waitAfterMs, 25.f, 0, 600000,
						"%d", ImGuiSliderFlags_AlwaysClamp))
					{
						action.waitAfterMs = static_cast<uint32_t>(waitAfterMs);
						m_bWorldNpcBehaviorDraftDirty = true;
					}
					if (ImGui::DragInt("Weight", &weight, 1.f, 1, 100000,
						"%d", ImGuiSliderFlags_AlwaysClamp))
					{
						action.weight = static_cast<uint32_t>(weight);
						m_bWorldNpcBehaviorDraftDirty = true;
					}
					m_bWorldNpcBehaviorDraftDirty |= ImGui::DragFloat(
						"Playback Rate", &action.playbackRate, 0.01f, 0.1f, 4.f,
						"%.2f", ImGuiSliderFlags_AlwaysClamp);
					m_bWorldNpcBehaviorDraftDirty |= ImGui::DragFloat(
						"Blend Seconds", &action.blendSeconds, 0.01f, 0.f, 2.f,
						"%.2f", ImGuiSliderFlags_AlwaysClamp);
					if (ImGui::SmallButton("Up Action") && 0u < actionIndex)
					{
						std::swap(behavior.actions[actionIndex - 1u],
							behavior.actions[actionIndex]);
						m_bWorldNpcBehaviorDraftDirty = true;
					}
					ImGui::SameLine();
					if (ImGui::SmallButton("Down Action") &&
						actionIndex + 1u < behavior.actions.size())
					{
						std::swap(behavior.actions[actionIndex],
							behavior.actions[actionIndex + 1u]);
						m_bWorldNpcBehaviorDraftDirty = true;
					}
					if (m_WorldNpcPreviews.end() != preview &&
						nullptr != preview->object && ImGui::SmallButton("Preview Action"))
						preview->object->Play_NetworkAction(
							action.clipName.c_str(), action.loop,
							action.playbackRate, action.blendSeconds);
					ImGui::SameLine();
					bool_t removed = false;
					if (ImGui::SmallButton("Delete Action"))
					{
						behavior.actions.erase(behavior.actions.begin() + actionIndex);
						m_bWorldNpcBehaviorDraftDirty = true;
						removed = true;
					}
					ImGui::Separator();
					ImGui::PopID();
					if (!removed) ++actionIndex;
				}
				ImGui::BeginDisabled(nullptr == behaviorModel ||
					behavior.actions.size() >= 32u);
				if (ImGui::Button("Add Action From Preview Clip"))
				{
					const char_t* clipName = behaviorModel->Get_AnimationName(
						behaviorModel->Get_CurrentAnimIndex());
					if (nullptr != clipName)
					{
						WORLD_NPC_ACTION action;
						for (uint32_t suffix = 1u; suffix <= 32u; ++suffix)
						{
							const std::string candidate =
								"npc.ambient." + std::to_string(suffix);
							const bool_t duplicate = std::any_of(
								behavior.actions.begin(), behavior.actions.end(),
								[&candidate](const WORLD_NPC_ACTION& existing)
								{
									return existing.actionId == candidate;
								});
							if (!duplicate)
							{
								action.actionId = candidate;
								break;
							}
						}
						if (action.actionId.empty())
						{
							m_WorldGameplayStatus =
								"No unused ambient action ID is available";
						}
						else
						{
							action.clipName = clipName;
							action.durationMs = 1000;
							behavior.actions.push_back(std::move(action));
							m_bWorldNpcBehaviorDraftDirty = true;
						}
					}
				}
				ImGui::EndDisabled();
			}

			const bool_t draftValid = !m_WorldNpcBehaviorDraft.has_value() ||
				CWorldGameplayDocument::Is_ValidNpcBehavior(*m_WorldNpcBehaviorDraft);
			ImGui::BeginDisabled(!m_bWorldNpcBehaviorDraftDirty || !draftValid);
			if (ImGui::Button("Apply NPC Behavior"))
				Apply_WorldNpcBehaviorDraft();
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(!m_bWorldNpcBehaviorDraftDirty);
			if (ImGui::Button("Revert NPC Behavior"))
			{
				m_WorldNpcBehaviorDraft = selected->npcBehavior;
				m_bWorldNpcBehaviorDraftDirty = false;
				m_bWorldNpcWaypointPickArmed = false;
			}
			ImGui::EndDisabled();
			if (!draftValid)
				ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
					"Draft is incomplete: moving modes need walk clip and valid route/radius.");
		}
		else
		{
			edited |= ImGui::Checkbox("Enabled", &staged.isEnabled);
		}
		if (edited)
		{
			const auto restoreQuickNpcDraft = [this, &staged]()
			{
				if (WORLD_PLACEMENT_KIND::NPC != staged.eKind)
					return;
				const WORLD_GAMEPLAY_PLACEMENT* restored =
					m_WorldGameplayDocument.Find(staged.placementId);
				m_WorldNpcBehaviorDraftPlacementId = staged.placementId;
				m_WorldNpcBehaviorDraft = nullptr != restored ?
					restored->npcBehavior : std::optional<WORLD_NPC_BEHAVIOR>{};
				m_bWorldNpcBehaviorDraftDirty = false;
			};
			const auto incomingLookReference = std::find_if(
				m_WorldGameplayDocument.Get_Placements().begin(),
				m_WorldGameplayDocument.Get_Placements().end(),
				[&staged](const WORLD_GAMEPLAY_PLACEMENT& placement)
				{
					return placement.placementId != staged.placementId &&
						placement.npcBehavior.has_value() &&
						placement.npcBehavior->lookTargetPlacementId ==
							staged.placementId;
				});
			if (WORLD_PLACEMENT_KIND::NPC == staged.eKind &&
				!staged.isEnabled && incomingLookReference !=
					m_WorldGameplayDocument.Get_Placements().end())
			{
				m_WorldGameplayStatus =
					"Cannot disable NPC referenced as look target by: " +
					incomingLookReference->placementId;
				if (quickNpcPresetEdited)
					restoreQuickNpcDraft();
			}
			else if (!CWorldGameplayDocument::Is_Valid(staged))
			{
				m_WorldGameplayStatus = "Gameplay edit rejected by validation";
				if (quickNpcPresetEdited)
					restoreQuickNpcDraft();
			}
			else if (WORLD_PLACEMENT_KIND::NPC == staged.eKind &&
				!Validate_WorldNpcBehaviorNavigation(
					staged, m_NavigationDocument, m_RuntimeBlockerDocument,
					m_WorldGameplayStatus))
			{
				if (quickNpcPresetEdited)
					restoreQuickNpcDraft();
			}
			else
			{
				CWorldGameplayDocument previous = m_WorldGameplayDocument;
				*selected = staged;
				m_WorldGameplayDocument.Mark_Edited();
				vector<TRIGGER_BOX_ENTRY> stagedBoxes;
				vector<NPC_PREVIEW_ENTRY> stagedPreviews;
				if (Stage_WorldTriggerBoxes(
						m_WorldGameplayDocument, stagedBoxes) &&
					Stage_WorldNpcPreviews(
						m_WorldGameplayDocument, stagedPreviews))
				{
					Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
					Remove_WorldNpcPreviews(m_WorldNpcPreviews);
					m_WorldTriggerBoxes = std::move(stagedBoxes);
					m_WorldNpcPreviews = std::move(stagedPreviews);
					m_bWorldGameplayDirty = true;
					if (quickNpcPresetEdited)
					{
						m_WorldNpcBehaviorDraftPlacementId = staged.placementId;
						m_WorldNpcBehaviorDraft = staged.npcBehavior;
						m_bWorldNpcBehaviorDraftDirty = false;
						m_WorldGameplayStatus =
							"Easy NPC preset applied; capture it for Quick NPC Brush or save Gameplay";
					}
					else
					{
						m_WorldGameplayStatus = "Gameplay placement edited";
					}
				}
				else
				{
					Remove_WorldTriggerBoxes(stagedBoxes);
					Remove_WorldNpcPreviews(stagedPreviews);
					m_WorldGameplayDocument = std::move(previous);
					if (quickNpcPresetEdited)
						restoreQuickNpcDraft();
				}
			}
			selected = m_WorldGameplayDocument.Find(
				m_SelectedWorldPlacementId);
		}
		ImGui::BeginDisabled(m_bWorldNpcBehaviorDraftDirty);
		const bool_t deletePlacement = nullptr != selected &&
			ImGui::Button("Delete Gameplay Placement");
		ImGui::EndDisabled();
		if (deletePlacement)
		{
			const std::string deletedId = selected->placementId;
			const auto reference = std::find_if(
				m_WorldGameplayDocument.Get_Placements().begin(),
				m_WorldGameplayDocument.Get_Placements().end(),
				[&deletedId](const WORLD_GAMEPLAY_PLACEMENT& placement)
				{
					return placement.npcBehavior.has_value() &&
						placement.npcBehavior->lookTargetPlacementId == deletedId;
				});
			if (reference != m_WorldGameplayDocument.Get_Placements().end())
			{
				m_WorldGameplayStatus =
					"Cannot delete placement referenced as NPC look target by: " +
					reference->placementId;
			}
			else
			{
				CWorldGameplayDocument previous = m_WorldGameplayDocument;
				if (m_WorldGameplayDocument.Remove(deletedId))
				{
					vector<TRIGGER_BOX_ENTRY> stagedBoxes;
					vector<NPC_PREVIEW_ENTRY> stagedPreviews;
					if (Stage_WorldTriggerBoxes(
							m_WorldGameplayDocument, stagedBoxes) &&
						Stage_WorldNpcPreviews(
							m_WorldGameplayDocument, stagedPreviews))
					{
						Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
						Remove_WorldNpcPreviews(m_WorldNpcPreviews);
						m_WorldTriggerBoxes = std::move(stagedBoxes);
						m_WorldNpcPreviews = std::move(stagedPreviews);
						m_SelectedWorldPlacementId.clear();
						m_bWorldGameplayDirty = true;
						m_WorldGameplayStatus =
							"Deleted gameplay placement: " + deletedId;
					}
					else
					{
						Remove_WorldTriggerBoxes(stagedBoxes);
						Remove_WorldNpcPreviews(stagedPreviews);
						m_WorldGameplayDocument = std::move(previous);
					}
				}
			}
		}
	}
	Render_SpawnGroupsPanel();
	ImGui::EndDisabled();
}

void Client::CMapTool::Render_SpawnGroupsPanel()
{
	ImGui::SeparatorText("Server Spawn Groups");
	ImGui::TextDisabled(
		"Anchors and waves stay in SpawnGroups.world.json; Trigger Boxes only reference a stable group ID.");
	if (ImGui::Button("Save Spawn Groups"))
		Save_SpawnGroups();
	ImGui::SameLine();
	if (ImGui::Button("Reload Spawn Groups"))
		Load_SpawnGroups();
	ImGui::SameLine();
	ImGui::Text("Revision: %u | Anchors: %zu | Groups: %zu%s",
		m_SpawnGroupDocument.Get_Revision(),
		m_SpawnGroupDocument.Get_Anchors().size(),
		m_SpawnGroupDocument.Get_Groups().size(),
		m_bSpawnGroupsDirty ? "  *unsaved" : "");

	ImGui::InputText("Anchor ID", m_SpawnAnchorId, std::size(m_SpawnAnchorId));
	if (ImGui::Button(m_bSpawnAnchorPlacementArmed ?
		"Cancel Spawn Anchor Pick" : "Place Spawn Anchor On Map"))
	{
		m_bSpawnAnchorPlacementArmed = !m_bSpawnAnchorPlacementArmed;
		m_bWorldGameplayPlacementArmed = false;
		m_bWorldTriggerTargetPickArmed = false;
		m_WorldGameplayStatus = m_bSpawnAnchorPlacementArmed ?
			"Spawn anchor armed: click a map surface" : "Spawn anchor pick cancelled";
	}
	if (m_bSpawnAnchorPlacementArmed)
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PICKING: click the exact monster spawn position");

	if (ImGui::BeginTable("SpawnAnchors", 2,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg,
		ImVec2(0.f, 130.f)))
	{
		ImGui::TableSetupColumn("Anchor ID");
		ImGui::TableSetupColumn("Position");
		ImGui::TableHeadersRow();
		for (const SPAWN_ANCHOR_RECORD& anchor :
			m_SpawnGroupDocument.Get_Anchors())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t selected = m_SelectedSpawnAnchorId == anchor.anchorId;
			if (ImGui::Selectable(anchor.anchorId.c_str(), selected,
				ImGuiSelectableFlags_SpanAllColumns))
				m_SelectedSpawnAnchorId = anchor.anchorId;
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2f, %.2f, %.2f", anchor.position.x,
				anchor.position.y, anchor.position.z);
		}
		ImGui::EndTable();
	}
	SPAWN_ANCHOR_RECORD* selectedAnchor =
		m_SpawnGroupDocument.Find_Anchor(m_SelectedSpawnAnchorId);
	if (nullptr != selectedAnchor)
	{
		bool_t edited = false;
		edited |= ImGui::DragFloat3("Selected Anchor Position",
			&selectedAnchor->position.x, 0.1f, -100000.f, 100000.f, "%.3f");
		edited |= ImGui::DragFloat("Selected Anchor Yaw",
			&selectedAnchor->yawDegrees, 0.5f, -360.f, 360.f, "%.2f");
		if (edited)
		{
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
			vector<TRIGGER_BOX_ENTRY> stagedBoxes;
			if (Stage_SpawnAnchorBoxes(m_SpawnGroupDocument, stagedBoxes))
			{
				Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
				m_SpawnAnchorBoxes = std::move(stagedBoxes);
			}
		}
		if (ImGui::Button("Delete Selected Spawn Anchor"))
		{
			if (m_SpawnGroupDocument.Remove_Anchor(
				m_SelectedSpawnAnchorId, m_WorldGameplayStatus))
			{
				m_SelectedSpawnAnchorId.clear();
				m_bSpawnGroupsDirty = true;
				vector<TRIGGER_BOX_ENTRY> stagedBoxes;
				if (Stage_SpawnAnchorBoxes(m_SpawnGroupDocument, stagedBoxes))
				{
					Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
					m_SpawnAnchorBoxes = std::move(stagedBoxes);
				}
			}
		}
	}

	ImGui::SeparatorText("Spawn Group Definition");
	ImGui::InputText("New Spawn Group ID", m_SpawnGroupId,
		std::size(m_SpawnGroupId));
	int maxAlive = static_cast<int>(m_iSpawnGroupMaxAlive);
	if (ImGui::InputInt("New Group Max Alive", &maxAlive))
		m_iSpawnGroupMaxAlive = static_cast<uint32_t>((std::max)(1, (std::min)(64, maxAlive)));
	if (ImGui::Button("Add Spawn Group"))
	{
		SPAWN_GROUP_RECORD group;
		group.spawnGroupId = m_SpawnGroupId;
		group.maxAlive = m_iSpawnGroupMaxAlive;
		if (m_SpawnGroupDocument.Add_Group(group, m_WorldGameplayStatus))
		{
			m_SelectedSpawnGroupId = group.spawnGroupId;
			m_bSpawnGroupsDirty = true;
		}
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Add at least one wave before Save.");

	if (ImGui::BeginTable("SpawnGroups", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg,
		ImVec2(0.f, 150.f)))
	{
		ImGui::TableSetupColumn("Group ID");
		ImGui::TableSetupColumn("Prerequisite");
		ImGui::TableSetupColumn("Max Alive");
		ImGui::TableSetupColumn("Waves");
		ImGui::TableHeadersRow();
		for (const SPAWN_GROUP_RECORD& group :
			m_SpawnGroupDocument.Get_Groups())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t selected = m_SelectedSpawnGroupId == group.spawnGroupId;
			if (ImGui::Selectable(group.spawnGroupId.c_str(), selected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedSpawnGroupId = group.spawnGroupId;
				m_SelectedSpawnWaveId.clear();
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(group.requiredCompletedGroupId.empty() ?
				"<none>" : group.requiredCompletedGroupId.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%u", group.maxAlive);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%zu", group.waves.size());
		}
		ImGui::EndTable();
	}

	SPAWN_GROUP_RECORD* selectedGroup =
		m_SpawnGroupDocument.Find_Group(m_SelectedSpawnGroupId);
	if (nullptr == selectedGroup)
		return;

	bool_t groupEdited = false;
	int selectedMaxAlive = static_cast<int>(selectedGroup->maxAlive);
	if (ImGui::InputInt("Selected Group Max Alive", &selectedMaxAlive))
	{
		selectedGroup->maxAlive = static_cast<uint32_t>(
			(std::max)(1, (std::min)(64, selectedMaxAlive)));
		groupEdited = true;
	}
	const char_t* prerequisitePreview = selectedGroup->requiredCompletedGroupId.empty() ?
		"<none>" : selectedGroup->requiredCompletedGroupId.c_str();
	if (ImGui::BeginCombo("Required Completed Group", prerequisitePreview))
	{
		if (ImGui::Selectable("<none>", selectedGroup->requiredCompletedGroupId.empty()))
		{
			selectedGroup->requiredCompletedGroupId.clear();
			groupEdited = true;
		}
		for (const SPAWN_GROUP_RECORD& group : m_SpawnGroupDocument.Get_Groups())
		{
			if (group.spawnGroupId == selectedGroup->spawnGroupId)
				continue;
			if (ImGui::Selectable(group.spawnGroupId.c_str(),
				selectedGroup->requiredCompletedGroupId == group.spawnGroupId))
			{
				selectedGroup->requiredCompletedGroupId = group.spawnGroupId;
				groupEdited = true;
			}
		}
		ImGui::EndCombo();
	}
	if (groupEdited)
	{
		m_SpawnGroupDocument.Mark_Edited();
		m_bSpawnGroupsDirty = true;
	}

	ImGui::InputText("New Wave ID", m_SpawnWaveId, std::size(m_SpawnWaveId));
	int startDelay = static_cast<int>(m_iSpawnWaveStartDelayMs);
	if (ImGui::InputInt("Wave Start Delay Ms", &startDelay))
		m_iSpawnWaveStartDelayMs = static_cast<uint32_t>((std::max)(0, startDelay));
	if (ImGui::Button("Add Wave"))
	{
		if (selectedGroup->waves.size() < CSpawnGroupDocument::MAX_WAVE_COUNT &&
			CSpawnGroupDocument::Is_ValidStableId(m_SpawnWaveId) &&
			std::none_of(selectedGroup->waves.begin(), selectedGroup->waves.end(),
				[&](const SPAWN_WAVE_RECORD& value) { return value.waveId == m_SpawnWaveId; }))
		{
			SPAWN_WAVE_RECORD wave;
			wave.waveId = m_SpawnWaveId;
			wave.startDelayMs = m_iSpawnWaveStartDelayMs;
			selectedGroup->waves.push_back(std::move(wave));
			m_SelectedSpawnWaveId = m_SpawnWaveId;
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
		}
		else
			m_WorldGameplayStatus = "Wave ID is invalid, duplicate, or over limit";
	}

	for (const SPAWN_WAVE_RECORD& wave : selectedGroup->waves)
	{
		ImGui::SameLine();
		if (ImGui::RadioButton(wave.waveId.c_str(), m_SelectedSpawnWaveId == wave.waveId))
			m_SelectedSpawnWaveId = wave.waveId;
	}
	const auto waveIter = std::find_if(selectedGroup->waves.begin(), selectedGroup->waves.end(),
		[&](const SPAWN_WAVE_RECORD& value) { return value.waveId == m_SelectedSpawnWaveId; });
	if (selectedGroup->waves.end() == waveIter)
		return;
	SPAWN_WAVE_RECORD& selectedWave = *waveIter;

	static constexpr const char_t* ARCHETYPES[] =
	{
		"MONSTER_VALTAN_PADD_01",
		"MONSTER_VALTAN_SJFC_00_4",
		"MONSTER_VALTAN_0019_05",
		"MINIBOSS_LUGARU"
	};
	int archetypeOption = static_cast<int>(m_iSpawnArchetypeOption);
	if (ImGui::Combo("Entry Archetype", &archetypeOption, ARCHETYPES,
		static_cast<int>(std::size(ARCHETYPES))))
		m_iSpawnArchetypeOption = static_cast<uint32_t>(archetypeOption);
	const char_t* anchorPreview = m_SelectedSpawnAnchorId.empty() ?
		"<select anchor>" : m_SelectedSpawnAnchorId.c_str();
	if (ImGui::BeginCombo("Entry Anchor", anchorPreview))
	{
		for (const SPAWN_ANCHOR_RECORD& anchor : m_SpawnGroupDocument.Get_Anchors())
			if (ImGui::Selectable(anchor.anchorId.c_str(),
				m_SelectedSpawnAnchorId == anchor.anchorId))
				m_SelectedSpawnAnchorId = anchor.anchorId;
		ImGui::EndCombo();
	}
	int entryCount = static_cast<int>(m_iSpawnEntryCount);
	int initialDelay = static_cast<int>(m_iSpawnInitialDelayMs);
	int interval = static_cast<int>(m_iSpawnIntervalMs);
	if (ImGui::InputInt("Entry Count", &entryCount))
		m_iSpawnEntryCount = static_cast<uint32_t>((std::max)(1, entryCount));
	if (ImGui::InputInt("Entry Initial Delay Ms", &initialDelay))
		m_iSpawnInitialDelayMs = static_cast<uint32_t>((std::max)(0, initialDelay));
	if (ImGui::InputInt("Entry Spawn Interval Ms", &interval))
		m_iSpawnIntervalMs = static_cast<uint32_t>((std::max)(0, interval));
	if (ImGui::Button("Add Entry To Selected Wave"))
	{
		if (selectedWave.entries.size() < CSpawnGroupDocument::MAX_ENTRY_COUNT &&
			nullptr != m_SpawnGroupDocument.Find_Anchor(m_SelectedSpawnAnchorId))
		{
			SPAWN_WAVE_ENTRY_RECORD entry;
			entry.archetypeId = ARCHETYPES[m_iSpawnArchetypeOption];
			entry.count = m_iSpawnEntryCount;
			entry.anchorId = m_SelectedSpawnAnchorId;
			entry.initialDelayMs = m_iSpawnInitialDelayMs;
			entry.spawnIntervalMs = m_iSpawnIntervalMs;
			selectedWave.entries.push_back(std::move(entry));
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
		}
		else
			m_WorldGameplayStatus = "Select a valid anchor or remove an entry first";
	}
	for (size_t index = 0; index < selectedWave.entries.size(); ++index)
	{
		const auto& entry = selectedWave.entries[index];
		ImGui::PushID(static_cast<int>(index));
		ImGui::Text("%zu. %s x%u @ %s (+%ums, every %ums)", index + 1u,
			entry.archetypeId.c_str(), entry.count, entry.anchorId.c_str(),
			entry.initialDelayMs, entry.spawnIntervalMs);
		ImGui::SameLine();
		if (ImGui::SmallButton("Remove"))
		{
			selectedWave.entries.erase(selectedWave.entries.begin() + index);
			m_SpawnGroupDocument.Mark_Edited();
			m_bSpawnGroupsDirty = true;
			ImGui::PopID();
			break;
		}
		ImGui::PopID();
	}
	if (ImGui::Button("Delete Selected Wave"))
	{
		selectedGroup->waves.erase(waveIter);
		m_SelectedSpawnWaveId.clear();
		m_SpawnGroupDocument.Mark_Edited();
		m_bSpawnGroupsDirty = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Selected Spawn Group"))
	{
		if (m_SpawnGroupDocument.Remove_Group(
			m_SelectedSpawnGroupId, m_WorldGameplayStatus))
		{
			m_SelectedSpawnGroupId.clear();
			m_SelectedSpawnWaveId.clear();
			m_bSpawnGroupsDirty = true;
		}
	}
}
