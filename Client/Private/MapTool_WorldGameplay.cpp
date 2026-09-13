#include "imgui.h"
#include "MapTool_Internal.h"
#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"
#include "Trigger_Box.h"
#include "Gameplay/WorldCollisionContract.h"
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

#include "MapTool_NavigationResources.h"



void Client::CMapTool::Render_WorldNpcRouteOverlay()
{
	if (!m_WorldNpcBehaviorDraft.has_value() ||
		WORLD_NPC_BEHAVIOR_MODE::PATROL != m_WorldNpcBehaviorDraft->eMode ||
		m_WorldNpcBehaviorDraft->waypoints.empty() || nullptr == m_pContext ||
		nullptr == m_pNavigationRenderResources ||
		nullptr == m_pNavigationRenderResources->pBatch ||
		nullptr == m_pNavigationRenderResources->pEffect ||
		nullptr == m_pNavigationRenderResources->pInputLayout)
	{
		return;
	}
	const WORLD_GAMEPLAY_PLACEMENT* placement =
		m_WorldGameplayDocument.Find(m_WorldNpcBehaviorDraftPlacementId);
	if (nullptr == placement || WORLD_PLACEMENT_KIND::NPC != placement->eKind)
		return;

	auto& resources = *m_pNavigationRenderResources;
	resources.pEffect->SetWorld(XMMatrixIdentity());
	resources.pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(resources.pInputLayout.Get());
	resources.pEffect->Apply(m_pContext.Get());

	const float4_t routeColor(0.1f, 0.9f, 1.f, 1.f);
	const float4_t markerColor(1.f, 0.85f, 0.1f, 1.f);
	const auto lifted = [](const float3_t& value)
	{
		return float3_t(value.x, value.y + 0.15f, value.z);
	};
	resources.pBatch->Begin();
	float3_t previous = lifted(placement->position);
	for (const WORLD_NPC_WAYPOINT& waypoint :
		m_WorldNpcBehaviorDraft->waypoints)
	{
		const float3_t current = lifted(waypoint.position);
		resources.pBatch->DrawLine(
			VertexPositionColor(previous, routeColor),
			VertexPositionColor(current, routeColor));
		constexpr f32_t markerRadius = 0.25f;
		resources.pBatch->DrawLine(
			VertexPositionColor(float3_t(current.x - markerRadius,
				current.y, current.z), markerColor),
			VertexPositionColor(float3_t(current.x + markerRadius,
				current.y, current.z), markerColor));
		resources.pBatch->DrawLine(
			VertexPositionColor(float3_t(current.x,
				current.y, current.z - markerRadius), markerColor),
			VertexPositionColor(float3_t(current.x,
				current.y, current.z + markerRadius), markerColor));
		previous = current;
	}
	if (WORLD_NPC_ROUTE_MODE::LOOP == m_WorldNpcBehaviorDraft->eRouteMode &&
		m_WorldNpcBehaviorDraft->waypoints.size() > 1u)
	{
		resources.pBatch->DrawLine(
			VertexPositionColor(previous, routeColor),
			VertexPositionColor(
				lifted(m_WorldNpcBehaviorDraft->waypoints.front().position),
				routeColor));
	}
	resources.pBatch->End();
}

void Client::CMapTool::Render_WorldNpcBatchOverlay()
{
	if (m_WorldNpcBatchDraft.empty() || nullptr == m_pContext ||
		nullptr == m_pNavigationRenderResources ||
		nullptr == m_pNavigationRenderResources->pBatch ||
		nullptr == m_pNavigationRenderResources->pEffect ||
		nullptr == m_pNavigationRenderResources->pInputLayout)
	{
		return;
	}

	auto& resources = *m_pNavigationRenderResources;
	resources.pEffect->SetWorld(XMMatrixIdentity());
	resources.pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(resources.pInputLayout.Get());
	resources.pEffect->Apply(m_pContext.Get());

	const float4_t ghostColor(1.f, 0.25f, 0.85f, 0.95f);
	const float4_t facingColor(0.25f, 1.f, 0.55f, 0.95f);
	constexpr f32_t markerRadius = 0.35f;
	constexpr f32_t facingLength = 0.75f;
	resources.pBatch->Begin();
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : m_WorldNpcBatchDraft)
	{
		const float3_t center(
			placement.position.x, placement.position.y + 0.2f,
			placement.position.z);
		resources.pBatch->DrawLine(
			VertexPositionColor(float3_t(center.x - markerRadius,
				center.y, center.z), ghostColor),
			VertexPositionColor(float3_t(center.x + markerRadius,
				center.y, center.z), ghostColor));
		resources.pBatch->DrawLine(
			VertexPositionColor(float3_t(center.x,
				center.y, center.z - markerRadius), ghostColor),
			VertexPositionColor(float3_t(center.x,
				center.y, center.z + markerRadius), ghostColor));
		const f32_t yawRadians =
			DirectX::XMConvertToRadians(placement.yawDegrees);
		resources.pBatch->DrawLine(
			VertexPositionColor(center, facingColor),
			VertexPositionColor(float3_t(
				center.x + std::sin(yawRadians) * facingLength,
				center.y,
				center.z + std::cos(yawRadians) * facingLength), facingColor));
	}
	resources.pBatch->End();
}

std::filesystem::path Client::CMapTool::Get_WorldGameplayPath() const
{
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	return nullptr != active &&
		EDITOR_GAMEPLAY_POLICY::REQUIRED == active->gameplayPolicy ?
		active->gameplayDocument : std::filesystem::path{};
}

std::filesystem::path Client::CMapTool::Get_SpawnGroupsPath() const
{
	const std::filesystem::path gameplayPath = Get_WorldGameplayPath();
	return gameplayPath.empty() ? std::filesystem::path{} :
		gameplayPath.parent_path() / L"SpawnGroups.world.json";
}

bool_t Client::CMapTool::Load_EncounterReference()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->encounterReference.empty())
	{
		m_EncounterReferenceStatus =
			"Active Area declares no encounter reference";
		return false;
	}

	CEncounterPatternReference staged;
	std::string status;
	if (!staged.Load(descriptor->encounterReference, status))
	{
		m_EncounterReferenceStatus = status;
		return false;
	}
	if (m_DestructionDocument.Is_Ready() &&
		!Validate_DestructionExternalReferences(
			m_DestructionDocument,
			m_DeployRuntime,
			m_RuntimeBlockerDocument,
			m_WorldGameplayDocument,
			staged,
			status))
	{
		m_EncounterReferenceStatus = status;
		return false;
	}
	m_EncounterReference = std::move(staged);
	m_EncounterReferenceStatus = status;
	if (nullptr == m_EncounterReference.Find_Pattern(
		m_SelectedDestructionPatternId))
	{
		m_SelectedDestructionPatternId.clear();
	}
	return true;
}

bool_t Client::CMapTool::Load_WorldGameplay()
{
	if (m_bWorldNpcBehaviorDraftDirty)
	{
		m_WorldGameplayStatus =
			"Apply or Revert the current NPC behavior draft before reloading";
		return false;
	}
	if (!m_WorldNpcBatchDraft.empty())
	{
		m_WorldGameplayStatus =
			"Confirm or Discard the staged NPC batch before reloading";
		return false;
	}
	const std::filesystem::path path = Get_WorldGameplayPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Gameplay load requires a ready map catalog";
		return false;
	}

	CWorldGameplayDocument stagedDocument;
	if (!stagedDocument.Load(
		path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
	{
		return false;
	}
	if (m_DestructionDocument.Is_Ready())
	{
		std::string status;
		if (!Validate_DestructionExternalReferences(
			m_DestructionDocument,
			m_DeployRuntime,
			m_RuntimeBlockerDocument,
			stagedDocument,
			m_EncounterReference,
			status))
		{
			m_WorldGameplayStatus = status;
			return false;
		}
	}
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	if (!Stage_WorldTriggerBoxes(stagedDocument, stagedBoxes))
		return false;
	vector<NPC_PREVIEW_ENTRY> stagedPreviews;
	if (!Stage_WorldNpcPreviews(stagedDocument, stagedPreviews))
	{
		Remove_WorldTriggerBoxes(stagedBoxes);
		return false;
	}

	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	m_WorldGameplayDocument = std::move(stagedDocument);
	m_WorldTriggerBoxes = std::move(stagedBoxes);
	m_WorldNpcPreviews = std::move(stagedPreviews);
	m_SelectedWorldPlacementId.clear();
	m_bWorldGameplayPlacementArmed = false;
	m_bWorldNpcContinuousPlacement = false;
	m_WorldNpcBrushPreset.reset();
	m_bWorldTriggerTargetPickArmed = false;
	m_bWorldNpcWaypointPickArmed = false;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_WorldNpcBehaviorDraftPlacementId.clear();
	m_WorldNpcBehaviorDraft.reset();
	m_bWorldNpcBehaviorDraftDirty = false;
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_bWorldGameplayDirty = false;
	return true;
}

bool_t Client::CMapTool::Save_WorldGameplay()
{
	if (!m_WorldNpcBatchDraft.empty())
	{
		m_WorldGameplayStatus =
			"Confirm or Discard the staged NPC batch before saving";
		return false;
	}
	const bool_t autoAppliedNpcBehavior = m_bWorldNpcBehaviorDraftDirty;
	if (autoAppliedNpcBehavior && !Apply_WorldNpcBehaviorDraft())
		return false;
	const std::filesystem::path path = Get_WorldGameplayPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Gameplay save requires a ready map catalog";
		return false;
	}
	if (m_DestructionDocument.Is_Ready())
	{
		std::string status;
		if (!Validate_CurrentDestructionReferences(status))
		{
			m_WorldGameplayStatus = status;
			m_DestructionStatus = status;
			return false;
		}
	}
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		m_WorldGameplayDocument.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind)
			continue;
		for (const WORLD_TRIGGER_EVENT& event : placement.triggerEvents)
		{
			if (WORLD_TRIGGER_EVENT_KIND::ACTIVATE_SPAWN_GROUP == event.eKind &&
				nullptr == m_SpawnGroupDocument.Find_Group(event.targetId))
			{
				m_WorldGameplayStatus =
					"Trigger references an unknown spawn group: " + event.targetId;
				return false;
			}
			if (WORLD_TRIGGER_EVENT_KIND::ACTIVATE_ENCOUNTER == event.eKind)
			{
				const WORLD_GAMEPLAY_PLACEMENT* target =
					m_WorldGameplayDocument.Find(event.targetId);
				if (nullptr == target || WORLD_PLACEMENT_KIND::BOSS != target->eKind ||
					target->isEnabled)
				{
					m_WorldGameplayStatus =
						"Encounter target must be a disabled boss placement: " + event.targetId;
					return false;
				}
			}
		}
	}
	if (!m_WorldGameplayDocument.Save(
		path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
	{
		return false;
	}
	m_bWorldGameplayDirty = false;
	if (autoAppliedNpcBehavior)
	{
		m_WorldGameplayStatus =
			"NPC behavior auto-applied; " + m_WorldGameplayStatus;
	}
	m_WorldGameplayStatus +=
		" Run Publish-WorldGameplay and restart Server for runtime changes.";
	return true;
}

bool_t Client::CMapTool::Load_SpawnGroups()
{
	const std::filesystem::path path = Get_SpawnGroupsPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Spawn group load requires a gameplay Area";
		return false;
	}
	CSpawnGroupDocument stagedDocument;
	if (!stagedDocument.Load(path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
		return false;
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	if (!Stage_SpawnAnchorBoxes(stagedDocument, stagedBoxes))
		return false;
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	m_SpawnGroupDocument = std::move(stagedDocument);
	m_SpawnAnchorBoxes = std::move(stagedBoxes);
	m_SelectedSpawnAnchorId.clear();
	m_SelectedSpawnGroupId.clear();
	m_SelectedSpawnWaveId.clear();
	m_bSpawnAnchorPlacementArmed = false;
	m_bSpawnGroupsDirty = false;
	return true;
}

bool_t Client::CMapTool::Save_SpawnGroups()
{
	const std::filesystem::path path = Get_SpawnGroupsPath();
	if (path.empty())
	{
		m_WorldGameplayStatus = "Spawn group save requires a gameplay Area";
		return false;
	}
	if (!m_SpawnGroupDocument.Save(
		path, m_Catalog.Get_AreaId(), m_WorldGameplayStatus))
		return false;
	m_bSpawnGroupsDirty = false;
	return true;
}

bool_t Client::CMapTool::Try_PlaceSpawnAnchor()
{
	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_WorldGameplayStatus = "Spawn anchor placement failed: map pick missed";
		return false;
	}
	SPAWN_ANCHOR_RECORD anchor;
	anchor.anchorId = m_SpawnAnchorId;
	anchor.position = position;
	if (!m_SpawnGroupDocument.Add_Anchor(anchor, m_WorldGameplayStatus))
		return false;
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	if (!Stage_SpawnAnchorBoxes(m_SpawnGroupDocument, stagedBoxes))
	{
		std::string removeStatus;
		m_SpawnGroupDocument.Remove_Anchor(anchor.anchorId, removeStatus);
		return false;
	}
	Remove_WorldTriggerBoxes(m_SpawnAnchorBoxes);
	m_SpawnAnchorBoxes = std::move(stagedBoxes);
	m_SelectedSpawnAnchorId = anchor.anchorId;
	m_bSpawnAnchorPlacementArmed = false;
	m_bSpawnGroupsDirty = true;
	return true;
}

bool_t Client::CMapTool::Stage_SpawnAnchorBoxes(
	const CSpawnGroupDocument& document,
	vector<TRIGGER_BOX_ENTRY>& outEntries)
{
	outEntries.clear();
	for (const SPAWN_ANCHOR_RECORD& anchor : document.Get_Anchors())
	{
		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = anchor.anchorId;
		desc.position = anchor.position;
		desc.halfExtents = float3_t(0.35f, 0.35f, 0.35f);
		desc.yawDegrees = anchor.yawDegrees;
		desc.isEnabled = true;
		desc.isCollisionBox = false;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_SpawnAnchors"),
			&desc,
			&gameObject)))
		{
			for (const TRIGGER_BOX_ENTRY& entry : outEntries)
				if (nullptr != entry.object)
					CGameInstance::Get().Remove_GameObject_from_Layer(
						m_iAuthoringLevelIndex, TEXT("Layer_SpawnAnchors"),
						static_pointer_cast<CGameObject>(entry.object));
			outEntries.clear();
			m_WorldGameplayStatus = "Spawn anchor presentation failed: " + anchor.anchorId;
			return false;
		}
		shared_ptr<CTrigger_Box> triggerBox =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == triggerBox)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex, TEXT("Layer_SpawnAnchors"), gameObject);
			return false;
		}
		triggerBox->Set_Selected(anchor.anchorId == m_SelectedSpawnAnchorId);
		triggerBox->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode);
		outEntries.push_back({ anchor.anchorId, std::move(triggerBox) });
	}
	return true;
}

std::string Client::CMapTool::Allocate_WorldNpcPlacementId(
	const std::string& archetypeId) const
{
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	const std::string areaId = nullptr != active ?
		active->areaId : m_Catalog.Get_AreaId();
	const std::string prefix = "npc." + Editor_AreaShortName(areaId) + "." +
		Editor_NpcArchetypeToken(archetypeId);
	for (uint32_t ordinal = 1u; ordinal <= 999999u; ++ordinal)
	{
		std::ostringstream candidate;
		candidate << prefix << '.' << std::setw(3) << std::setfill('0') <<
			ordinal;
		if (nullptr == m_WorldGameplayDocument.Find(candidate.str()))
			return candidate.str();
	}
	return {};
}

bool_t Client::CMapTool::Try_PlaceWorldGameplay()
{
	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_WorldGameplayStatus = "Gameplay placement failed: map pick missed";
		return false;
	}

	const bool_t continuousNpcBrush =
		m_bWorldGameplayPlacementArmed &&
		m_bWorldNpcContinuousPlacement &&
		WORLD_PLACEMENT_KIND::NPC == m_eWorldPlacementKind;
	WORLD_GAMEPLAY_PLACEMENT placement;
	placement.eKind = m_eWorldPlacementKind;
	placement.position = position;
	placement.yawDegrees = 0.f;
	if (continuousNpcBrush)
	{
		placement.archetypeId = m_WorldNpcBrushPreset.has_value() ?
			m_WorldNpcBrushPreset->archetypeId : m_WorldArchetypeId;
		placement.placementId = Allocate_WorldNpcPlacementId(
			placement.archetypeId);
		if (placement.placementId.empty())
		{
			m_WorldGameplayStatus =
				"Quick NPC Brush failed: no automatic placement ID is available";
			return false;
		}

		size_t worldEntityCount = 0u;
		for (const WORLD_GAMEPLAY_PLACEMENT& existing :
			m_WorldGameplayDocument.Get_Placements())
		{
			if (existing.isEnabled &&
				(WORLD_PLACEMENT_KIND::NPC == existing.eKind ||
				 WORLD_PLACEMENT_KIND::BOSS == existing.eKind))
			{
				++worldEntityCount;
			}
		}
		if (worldEntityCount >= 256u)
		{
			m_WorldGameplayStatus =
				"Quick NPC Brush failed: world entity budget is already 256";
			return false;
		}

		placement.isEnabled = true;
		const uint32_t placementSeed =
			Editor_StableSeed(placement.placementId);
		if (m_bWorldNpcBrushRandomYaw)
			placement.yawDegrees = static_cast<f32_t>(
				placementSeed % 36000u) * 0.01f;
		if (m_WorldNpcBrushPreset.has_value())
		{
			const WORLD_GAMEPLAY_PLACEMENT& source =
				*m_WorldNpcBrushPreset;
			placement.npcIdleClip = source.npcIdleClip;
			placement.npcBehavior = source.npcBehavior;
			if (placement.npcBehavior.has_value())
			{
				WORLD_NPC_BEHAVIOR& behavior = *placement.npcBehavior;
				behavior.lookTargetPlacementId.clear();
				behavior.randomSeed = placementSeed;
				for (WORLD_NPC_WAYPOINT& waypoint : behavior.waypoints)
				{
					waypoint.position.x += placement.position.x - source.position.x;
					waypoint.position.z += placement.position.z - source.position.z;
					int32_t cellX = 0;
					int32_t cellZ = 0;
					if (!m_NavigationDocument.World_ToCell(
							XMLoadFloat3(&waypoint.position), cellX, cellZ))
					{
						m_WorldGameplayStatus =
							"Quick NPC Brush failed: translated patrol waypoint is outside navigation";
						return false;
					}
					const uint32_t cellIndex =
						m_NavigationDocument.To_Index(cellX, cellZ);
					if (!m_NavigationDocument.Has_ResolvedHeight(cellIndex) ||
						NAVGRID_AUTHORING_CELL_STATE::WALKABLE !=
							m_NavigationDocument.Get_CellState(cellIndex))
					{
						m_WorldGameplayStatus =
							"Quick NPC Brush failed: translated patrol waypoint is not walkable";
						return false;
					}
					waypoint.position.y =
						m_NavigationDocument.Get_CellHeight(cellIndex);
				}
			}
		}
		if (!CWorldGameplayDocument::Is_Valid(placement))
		{
			m_WorldGameplayStatus =
				"Quick NPC Brush failed: preset validation rejected the placement";
			return false;
		}
		if (!Validate_WorldNpcBehaviorNavigation(
				placement, m_NavigationDocument, m_RuntimeBlockerDocument,
				m_WorldGameplayStatus))
		{
			return false;
		}
	}
	else
	{
		std::string placementId = m_WorldPlacementId;
		if (!placementId.empty() &&
			nullptr != m_WorldGameplayDocument.Find(placementId))
		{
			std::string candidate;
			for (uint32_t suffix = 2u; suffix < 1000u; ++suffix)
			{
				candidate = placementId + "." + std::to_string(suffix);
				if (nullptr == m_WorldGameplayDocument.Find(candidate))
					break;
				candidate.clear();
			}
			if (candidate.empty())
			{
				m_WorldGameplayStatus =
					"Gameplay placement failed: no free placement ID";
				return false;
			}
			placementId = std::move(candidate);
		}
		placement.placementId = placementId;
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX == placement.eKind)
		{
			placement.isEnabled = false;
			placement.halfExtents = m_WorldTriggerHalfExtents;
			placement.isTriggerOnce = m_bWorldTriggerOnce;
		}
		else if (WORLD_PLACEMENT_KIND::COLLISION_BOX == placement.eKind)
		{
			placement.isEnabled = true;
			placement.halfExtents = m_WorldTriggerHalfExtents;
		}
		else
		{
			placement.archetypeId = m_WorldArchetypeId;
			placement.encounterId = m_WorldEncounterId;
			placement.isEnabled = true;
		}
	}

	CWorldGameplayDocument previous = m_WorldGameplayDocument;
	if (!m_WorldGameplayDocument.Add(placement, m_WorldGameplayStatus))
		return false;
	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	vector<NPC_PREVIEW_ENTRY> stagedPreviews;
	if (!Stage_WorldTriggerBoxes(m_WorldGameplayDocument, stagedBoxes) ||
		!Stage_WorldNpcPreviews(m_WorldGameplayDocument, stagedPreviews))
	{
		Remove_WorldTriggerBoxes(stagedBoxes);
		Remove_WorldNpcPreviews(stagedPreviews);
		m_WorldGameplayDocument = std::move(previous);
		return false;
	}

	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	m_WorldTriggerBoxes = std::move(stagedBoxes);
	m_WorldNpcPreviews = std::move(stagedPreviews);
	m_SelectedWorldPlacementId = placement.placementId;
	m_bWorldGameplayPlacementArmed = continuousNpcBrush;
	m_bWorldGameplayDirty = true;
	if (continuousNpcBrush)
	{
		m_WorldGameplayStatus = "Quick NPC Brush placed: " +
			placement.placementId + "; click again or press Esc to stop";
	}
	return true;
}

bool_t Client::CMapTool::Try_PickWorldTriggerTarget()
{
	WORLD_GAMEPLAY_PLACEMENT* placement =
		m_WorldGameplayDocument.Find(m_SelectedWorldPlacementId);
	if (nullptr == placement ||
		WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement->eKind)
	{
		m_bWorldTriggerTargetPickArmed = false;
		m_WorldGameplayStatus = "Move target pick requires a selected Trigger Box";
		return false;
	}

	float3_t target{};
	if (!Try_PickPlacementPosition(target))
	{
		m_WorldGameplayStatus = "Move target pick failed: map surface was not hit";
		return false;
	}

	WORLD_GAMEPLAY_PLACEMENT staged = *placement;
	if (1u != staged.triggerEvents.size() ||
		WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER != staged.triggerEvents.front().eKind)
	{
		WORLD_TRIGGER_EVENT action{};
		action.eKind = WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER;
		action.durationSeconds = 0.8f;
		staged.triggerEvents.assign(1u, action);
	}
	staged.triggerEvents.front().targetPosition = target;
	if (!CWorldGameplayDocument::Is_Valid(staged))
	{
		m_WorldGameplayStatus = "Move target pick produced an invalid trigger action";
		return false;
	}

	*placement = std::move(staged);
	m_WorldGameplayDocument.Mark_Edited();
	m_bWorldGameplayDirty = true;
	m_bWorldTriggerTargetPickArmed = false;
	m_WorldGameplayStatus = "Move target stored. Enable the Trigger Box and save gameplay.";
	return true;
}

void Client::CMapTool::Sync_WorldNpcBehaviorDraft(
	const WORLD_GAMEPLAY_PLACEMENT& placement)
{
	if (m_WorldNpcBehaviorDraftPlacementId == placement.placementId)
		return;
	m_WorldNpcBehaviorDraftPlacementId = placement.placementId;
	m_WorldNpcBehaviorDraft = placement.npcBehavior;
	m_bWorldNpcBehaviorDraftDirty = false;
	m_bWorldNpcWaypointPickArmed = false;
}

bool_t Client::CMapTool::Apply_WorldNpcBehaviorDraft()
{
	WORLD_GAMEPLAY_PLACEMENT* placement =
		m_WorldGameplayDocument.Find(m_WorldNpcBehaviorDraftPlacementId);
	if (nullptr == placement || WORLD_PLACEMENT_KIND::NPC != placement->eKind)
	{
		m_WorldGameplayStatus = "NPC behavior apply requires the selected NPC";
		return false;
	}

	WORLD_GAMEPLAY_PLACEMENT stagedPlacement = *placement;
	stagedPlacement.npcBehavior = m_WorldNpcBehaviorDraft;
	if (!CWorldGameplayDocument::Is_Valid(stagedPlacement))
	{
		m_WorldGameplayStatus = "NPC behavior draft failed contract validation";
		return false;
	}
	if (stagedPlacement.npcBehavior.has_value() &&
		!stagedPlacement.npcBehavior->lookTargetPlacementId.empty())
	{
		const WORLD_GAMEPLAY_PLACEMENT* target = m_WorldGameplayDocument.Find(
			stagedPlacement.npcBehavior->lookTargetPlacementId);
		if (nullptr == target || WORLD_PLACEMENT_KIND::NPC != target->eKind ||
			!target->isEnabled ||
			target->placementId == stagedPlacement.placementId)
		{
			m_WorldGameplayStatus =
				"NPC look target must reference another enabled NPC";
			return false;
		}
	}
	if (!Validate_WorldNpcBehaviorNavigation(
		stagedPlacement, m_NavigationDocument, m_RuntimeBlockerDocument,
		m_WorldGameplayStatus))
	{
		return false;
	}

	if (stagedPlacement.npcBehavior.has_value())
	{
		const auto preview = std::find_if(
			m_WorldNpcPreviews.begin(), m_WorldNpcPreviews.end(),
			[&](const NPC_PREVIEW_ENTRY& entry)
			{
				return entry.placementId == stagedPlacement.placementId;
			});
		const shared_ptr<CModel> model =
			m_WorldNpcPreviews.end() != preview && nullptr != preview->object ?
			preview->object->Get_Model() : nullptr;
		if (nullptr == model)
		{
			m_WorldGameplayStatus =
				"NPC behavior apply requires an admitted preview model";
			return false;
		}
		const auto hasClip = [&model](const std::string& clipName)
		{
			if (clipName.empty())
				return true;
			for (uint32_t index = 0; index < model->Get_NumAnimations(); ++index)
			{
				const char_t* candidate = model->Get_AnimationName(index);
				if (nullptr != candidate && clipName == candidate)
					return true;
			}
			return false;
		};
		if (!hasClip(stagedPlacement.npcBehavior->walkClip) ||
			!std::all_of(stagedPlacement.npcBehavior->actions.begin(),
				stagedPlacement.npcBehavior->actions.end(),
				[&hasClip](const WORLD_NPC_ACTION& action)
				{
					return hasClip(action.clipName);
				}))
		{
			m_WorldGameplayStatus =
				"NPC behavior contains a clip missing from this animation set";
			return false;
		}
	}

	CWorldGameplayDocument previous = m_WorldGameplayDocument;
	*placement = std::move(stagedPlacement);
	m_WorldGameplayDocument.Mark_Edited();
	vector<NPC_PREVIEW_ENTRY> stagedPreviews;
	if (!Stage_WorldNpcPreviews(m_WorldGameplayDocument, stagedPreviews))
	{
		m_WorldGameplayDocument = std::move(previous);
		return false;
	}
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	m_WorldNpcPreviews = std::move(stagedPreviews);
	m_bWorldGameplayDirty = true;
	m_bWorldNpcBehaviorDraftDirty = false;
	m_bWorldNpcWaypointPickArmed = false;
	m_WorldGameplayStatus = "NPC behavior applied; save Gameplay to publish it";
	return true;
}

bool_t Client::CMapTool::Try_PickWorldNpcWaypoint()
{
	WORLD_GAMEPLAY_PLACEMENT* placement =
		m_WorldGameplayDocument.Find(m_WorldNpcBehaviorDraftPlacementId);
	if (nullptr == placement || WORLD_PLACEMENT_KIND::NPC != placement->eKind ||
		!m_WorldNpcBehaviorDraft.has_value() ||
		WORLD_NPC_BEHAVIOR_MODE::PATROL != m_WorldNpcBehaviorDraft->eMode)
	{
		m_bWorldNpcWaypointPickArmed = false;
		m_WorldGameplayStatus = "Waypoint pick requires a Patrol behavior draft";
		return false;
	}
	if (m_WorldNpcBehaviorDraft->waypoints.size() >= 64u)
	{
		m_bWorldNpcWaypointPickArmed = false;
		m_WorldGameplayStatus = "NPC waypoint limit is 64";
		return false;
	}

	float3_t picked{};
	if (!Try_PickPlacementPosition(picked))
	{
		m_WorldGameplayStatus = "Waypoint pick missed the map surface";
		return false;
	}
	if (!m_NavigationDocument.Is_Ready())
	{
		m_WorldGameplayStatus = "Waypoint pick requires loaded navigation";
		return false;
	}
	int32_t cellX = 0;
	int32_t cellZ = 0;
	if (!m_NavigationDocument.World_ToCell(XMLoadFloat3(&picked), cellX, cellZ))
	{
		m_WorldGameplayStatus = "Waypoint is outside navigation bounds";
		return false;
	}
	const uint32_t cellIndex = m_NavigationDocument.To_Index(cellX, cellZ);
	if (!m_NavigationDocument.Has_ResolvedHeight(cellIndex) ||
		NAVGRID_AUTHORING_CELL_STATE::WALKABLE !=
			m_NavigationDocument.Get_CellState(cellIndex))
	{
		m_WorldGameplayStatus = "Waypoint must be on a walkable navigation cell";
		return false;
	}
	picked.y = m_NavigationDocument.Get_CellHeight(cellIndex);

	WORLD_NPC_WAYPOINT waypoint;
	for (uint32_t suffix = 1; suffix <= 64u; ++suffix)
	{
		std::ostringstream id;
		id << "wp." << std::setw(2) << std::setfill('0') << suffix;
		const bool_t duplicate = std::any_of(
			m_WorldNpcBehaviorDraft->waypoints.begin(),
			m_WorldNpcBehaviorDraft->waypoints.end(),
			[&id](const WORLD_NPC_WAYPOINT& existing)
			{
				return existing.waypointId == id.str();
			});
		if (!duplicate)
		{
			waypoint.waypointId = id.str();
			break;
		}
	}
	if (waypoint.waypointId.empty())
	{
		m_WorldGameplayStatus = "No stable waypoint ID is available";
		return false;
	}
	waypoint.position = picked;
	waypoint.waitMs = 1000;
	m_WorldNpcBehaviorDraft->waypoints.push_back(std::move(waypoint));
	m_bWorldNpcBehaviorDraftDirty = true;
	m_bWorldNpcWaypointPickArmed = false;
	m_WorldGameplayStatus = "Waypoint added to the NPC behavior draft";
	return true;
}

bool_t Client::CMapTool::Try_PickWorldNpcBatchCenter()
{
	float3_t picked{};
	if (!Try_PickPlacementPosition(picked))
	{
		m_WorldGameplayStatus = "NPC batch center pick missed the map surface";
		return false;
	}
	if (!m_NavigationDocument.Is_Ready())
	{
		m_WorldGameplayStatus = "NPC batch placement requires loaded navigation";
		return false;
	}
	int32_t cellX = 0;
	int32_t cellZ = 0;
	if (!m_NavigationDocument.World_ToCell(XMLoadFloat3(&picked), cellX, cellZ))
	{
		m_WorldGameplayStatus = "NPC batch center is outside navigation bounds";
		return false;
	}
	const uint32_t cellIndex = m_NavigationDocument.To_Index(cellX, cellZ);
	if (!m_NavigationDocument.Has_ResolvedHeight(cellIndex) ||
		NAVGRID_AUTHORING_CELL_STATE::WALKABLE !=
			m_NavigationDocument.Get_CellState(cellIndex))
	{
		m_WorldGameplayStatus = "NPC batch center must be on walkable navigation";
		return false;
	}
	picked.y = m_NavigationDocument.Get_CellHeight(cellIndex);
	m_WorldNpcBatchCenter = picked;
	m_bWorldNpcBatchCenterValid = true;
	m_bWorldNpcBatchCenterPickArmed = false;
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_WorldGameplayStatus = "NPC batch center stored";
	return true;
}

bool_t Client::CMapTool::Place_WorldNpcBatch()
{
	std::vector<std::string> archetypePool(
		m_WorldNpcBatchArchetypePool.begin(),
		m_WorldNpcBatchArchetypePool.end());
	std::sort(archetypePool.begin(), archetypePool.end());
	if (archetypePool.empty() ||
		!std::all_of(archetypePool.begin(), archetypePool.end(),
			[](const std::string& archetypeId)
			{
				return nullptr != CActorCatalog::Find_Npc(archetypeId);
			}) || !m_bWorldNpcBatchCenterValid ||
		!m_NavigationDocument.Is_Ready() || 0u == m_iWorldNpcBatchCount ||
		m_iWorldNpcBatchCount > 128u || m_iWorldNpcBatchSeed == 0u ||
		m_fWorldNpcBatchRadius < 0.5f || m_fWorldNpcBatchRadius > 100.f ||
		m_fWorldNpcBatchMinimumSpacing < 0.25f ||
		m_fWorldNpcBatchMinimumSpacing > 20.f)
	{
		m_WorldGameplayStatus = "NPC batch settings or selected archetype are invalid";
		return false;
	}

	size_t worldEntityCount = 0;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		m_WorldGameplayDocument.Get_Placements())
	{
		if (placement.isEnabled &&
			(WORLD_PLACEMENT_KIND::NPC == placement.eKind ||
			 WORLD_PLACEMENT_KIND::BOSS == placement.eKind))
		{
			++worldEntityCount;
		}
	}
	if (worldEntityCount + m_iWorldNpcBatchCount > 256u)
	{
		m_WorldGameplayStatus = "NPC batch exceeds the 256 world entity budget";
		return false;
	}
	const WORLD_GAMEPLAY_PLACEMENT* behaviorSource = nullptr;
	if (m_bWorldNpcBatchCopySelectedBehavior)
	{
		if (m_bWorldNpcBehaviorDraftDirty)
		{
			m_WorldGameplayStatus =
				"Apply or Revert the selected NPC behavior before staging a batch";
			return false;
		}
		behaviorSource = m_WorldGameplayDocument.Find(m_SelectedWorldPlacementId);
		if (nullptr == behaviorSource ||
			WORLD_PLACEMENT_KIND::NPC != behaviorSource->eKind ||
			!behaviorSource->npcBehavior.has_value())
		{
			m_WorldGameplayStatus =
				"Batch behavior copy requires a selected NPC with applied behavior";
			return false;
		}
	}

	std::vector<float3_t> accepted;
	accepted.reserve(m_iWorldNpcBatchCount);
	uint32_t rngState = m_iWorldNpcBatchSeed;
	auto nextRandom = [&rngState]()
	{
		rngState ^= rngState << 13;
		rngState ^= rngState >> 17;
		rngState ^= rngState << 5;
		return static_cast<f32_t>((rngState >> 8) * (1.0 / 16777216.0));
	};
	using namespace LostArk::Shared::WorldCollision;
	const f32_t minimumNpcCenterSpacing = (std::max)(
		m_fWorldNpcBatchMinimumSpacing, PLAYER_HALF_EXTENT_X * 2.f);
	const f32_t minimumNpcCenterSpacingSquared =
		minimumNpcCenterSpacing * minimumNpcCenterSpacing;
	const f32_t collisionClearance = (std::max)(
		m_fWorldNpcBatchMinimumSpacing, PLAYER_HALF_EXTENT_X);
	const uint32_t maximumAttempts = m_iWorldNpcBatchCount * 256u;
	for (uint32_t attempt = 0;
		attempt < maximumAttempts && accepted.size() < m_iWorldNpcBatchCount;
		++attempt)
	{
		const f32_t radius = std::sqrt(nextRandom()) * m_fWorldNpcBatchRadius;
		const f32_t angle = nextRandom() * DirectX::XM_2PI;
		float3_t candidate(
			m_WorldNpcBatchCenter.x + std::cos(angle) * radius,
			m_WorldNpcBatchCenter.y,
			m_WorldNpcBatchCenter.z + std::sin(angle) * radius);
		int32_t cellX = 0;
		int32_t cellZ = 0;
		if (!m_NavigationDocument.World_ToCell(
			XMLoadFloat3(&candidate), cellX, cellZ))
		{
			continue;
		}
		const uint32_t cellIndex = m_NavigationDocument.To_Index(cellX, cellZ);
		if (!m_NavigationDocument.Has_ResolvedHeight(cellIndex) ||
			NAVGRID_AUTHORING_CELL_STATE::WALKABLE !=
				m_NavigationDocument.Get_CellState(cellIndex))
		{
			continue;
		}
		candidate.y = m_NavigationDocument.Get_CellHeight(cellIndex);

		bool_t overlaps = false;
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			m_WorldGameplayDocument.Get_Placements())
		{
			if (WORLD_PLACEMENT_KIND::NPC == placement.eKind &&
				placement.isEnabled &&
				NpcBatchBodiesOverlapVertically(
					candidate.y, placement.position.y))
			{
				const f32_t dx = candidate.x - placement.position.x;
				const f32_t dz = candidate.z - placement.position.z;
				if (dx * dx + dz * dz < minimumNpcCenterSpacingSquared)
				{
					overlaps = true;
					break;
				}
			}
			else if (WORLD_PLACEMENT_KIND::COLLISION_BOX == placement.eKind &&
				placement.isEnabled &&
				NpcBatchBodyOverlapsCollisionHeight(candidate.y, placement))
			{
				const f32_t radians = -DirectX::XMConvertToRadians(placement.yawDegrees);
				const f32_t dx = candidate.x - placement.position.x;
				const f32_t dz = candidate.z - placement.position.z;
				const f32_t localX = dx * std::cos(radians) - dz * std::sin(radians);
				const f32_t localZ = dx * std::sin(radians) + dz * std::cos(radians);
				if (std::abs(localX) <=
						placement.halfExtents.x + collisionClearance &&
					std::abs(localZ) <=
						placement.halfExtents.z + collisionClearance)
				{
					overlaps = true;
					break;
				}
			}
		}
		if (!overlaps)
		{
			for (const float3_t& existing : accepted)
			{
				if (!NpcBatchBodiesOverlapVertically(candidate.y, existing.y))
					continue;
				const f32_t dx = candidate.x - existing.x;
				const f32_t dz = candidate.z - existing.z;
				if (dx * dx + dz * dz < minimumNpcCenterSpacingSquared)
				{
					overlaps = true;
					break;
				}
			}
		}
		if (!overlaps)
			accepted.push_back(candidate);
	}
	if (accepted.size() != m_iWorldNpcBatchCount)
	{
		m_WorldGameplayStatus =
			"NPC batch could not satisfy navigation and minimum spacing; nothing changed";
		return false;
	}

	CWorldGameplayDocument stagedDocument = m_WorldGameplayDocument;
	std::vector<WORLD_GAMEPLAY_PLACEMENT> stagedBatch;
	stagedBatch.reserve(accepted.size());
	for (size_t index = 0; index < accepted.size(); ++index)
	{
		std::ostringstream id;
		id << m_WorldNpcBatchIdPrefix << '.' << std::setw(3) <<
			std::setfill('0') << index + 1u;
		WORLD_GAMEPLAY_PLACEMENT placement;
		placement.placementId = id.str();
		placement.eKind = WORLD_PLACEMENT_KIND::NPC;
		const size_t archetypeIndex = (std::min)(archetypePool.size() - 1u,
			static_cast<size_t>(nextRandom() * archetypePool.size()));
		placement.archetypeId = archetypePool[archetypeIndex];
		placement.position = accepted[index];
		placement.yawDegrees = m_bWorldNpcBatchRandomYaw ?
			nextRandom() * 360.f : 0.f;
		placement.isEnabled = true;
		if (nullptr != behaviorSource)
		{
			placement.npcBehavior = behaviorSource->npcBehavior;
			WORLD_NPC_BEHAVIOR& behavior = *placement.npcBehavior;
			behavior.lookTargetPlacementId.clear();
			behavior.randomSeed = m_iWorldNpcBatchSeed +
				static_cast<uint32_t>(index + 1u);
			if (0u == behavior.randomSeed)
				behavior.randomSeed = 1u;
			for (WORLD_NPC_WAYPOINT& waypoint : behavior.waypoints)
			{
				waypoint.position.x +=
					placement.position.x - behaviorSource->position.x;
				waypoint.position.z +=
					placement.position.z - behaviorSource->position.z;
				int32_t waypointCellX = 0;
				int32_t waypointCellZ = 0;
				if (!m_NavigationDocument.World_ToCell(
						XMLoadFloat3(&waypoint.position),
						waypointCellX, waypointCellZ))
				{
					m_WorldGameplayStatus =
						"Translated batch patrol waypoint is outside navigation";
					return false;
				}
				const uint32_t waypointCellIndex =
					m_NavigationDocument.To_Index(waypointCellX, waypointCellZ);
				if (!m_NavigationDocument.Has_ResolvedHeight(waypointCellIndex) ||
					NAVGRID_AUTHORING_CELL_STATE::WALKABLE !=
						m_NavigationDocument.Get_CellState(waypointCellIndex))
				{
					m_WorldGameplayStatus =
						"Translated batch patrol waypoint is not walkable";
					return false;
				}
				waypoint.position.y =
					m_NavigationDocument.Get_CellHeight(waypointCellIndex);
			}
		}
		if (!stagedDocument.Add(placement, m_WorldGameplayStatus))
		{
			m_WorldGameplayStatus += "; batch transaction rolled back";
			return false;
		}
		stagedBatch.push_back(std::move(placement));
	}
	vector<NPC_PREVIEW_ENTRY> admittedPreviews;
	if (!Stage_WorldNpcPreviews(stagedDocument, admittedPreviews))
	{
		Remove_WorldNpcPreviews(admittedPreviews);
		m_WorldGameplayStatus +=
			"; batch behavior is incompatible with at least one target animation set";
		return false;
	}
	Remove_WorldNpcPreviews(admittedPreviews);
	m_WorldNpcBatchDraft = std::move(stagedBatch);
	m_iWorldNpcBatchDraftBaseRevision =
		m_WorldGameplayDocument.Get_Revision();
	m_WorldGameplayStatus = "Staged NPC batch ghost: " +
		std::to_string(m_WorldNpcBatchDraft.size()) +
		" placement(s); confirm or discard";
	return true;
}

bool_t Client::CMapTool::Commit_WorldNpcBatch()
{
	if (m_WorldNpcBatchDraft.empty() ||
		m_iWorldNpcBatchDraftBaseRevision !=
			m_WorldGameplayDocument.Get_Revision())
	{
		m_WorldGameplayStatus =
			"NPC batch ghost is stale; discard it and stage again";
		return false;
	}

	CWorldGameplayDocument stagedDocument = m_WorldGameplayDocument;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : m_WorldNpcBatchDraft)
	{
		if (!stagedDocument.Add(placement, m_WorldGameplayStatus))
		{
			m_WorldGameplayStatus += "; batch transaction rolled back";
			return false;
		}
	}

	vector<TRIGGER_BOX_ENTRY> stagedBoxes;
	vector<NPC_PREVIEW_ENTRY> stagedPreviews;
	if (!Stage_WorldTriggerBoxes(stagedDocument, stagedBoxes) ||
		!Stage_WorldNpcPreviews(stagedDocument, stagedPreviews))
	{
		Remove_WorldTriggerBoxes(stagedBoxes);
		Remove_WorldNpcPreviews(stagedPreviews);
		m_WorldGameplayStatus = "NPC batch presentation failed; transaction rolled back";
		return false;
	}
	const size_t expectedNpcCount = static_cast<size_t>(std::count_if(
		stagedDocument.Get_Placements().begin(), stagedDocument.Get_Placements().end(),
		[](const WORLD_GAMEPLAY_PLACEMENT& placement)
		{
			return WORLD_PLACEMENT_KIND::NPC == placement.eKind;
		}));
	if (stagedPreviews.size() != expectedNpcCount)
	{
		Remove_WorldTriggerBoxes(stagedBoxes);
		Remove_WorldNpcPreviews(stagedPreviews);
		m_WorldGameplayStatus = "NPC batch preview admission failed; transaction rolled back";
		return false;
	}

	Remove_WorldTriggerBoxes(m_WorldTriggerBoxes);
	Remove_WorldNpcPreviews(m_WorldNpcPreviews);
	m_WorldGameplayDocument = std::move(stagedDocument);
	m_WorldTriggerBoxes = std::move(stagedBoxes);
	m_WorldNpcPreviews = std::move(stagedPreviews);
	m_bWorldGameplayDirty = true;
	const size_t committedCount = m_WorldNpcBatchDraft.size();
	m_WorldNpcBatchDraft.clear();
	m_iWorldNpcBatchDraftBaseRevision = 0;
	m_WorldGameplayStatus = "Committed NPC batch: " +
		std::to_string(committedCount) + " placement(s)";
	return true;
}

bool_t Client::CMapTool::Stage_WorldTriggerBoxes(
	const CWorldGameplayDocument& document,
	vector<TRIGGER_BOX_ENTRY>& outEntries)
{
	outEntries.clear();
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind &&
			WORLD_PLACEMENT_KIND::COLLISION_BOX != placement.eKind)
			continue;

		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = placement.placementId;
		desc.position = placement.position;
		desc.halfExtents = placement.halfExtents;
		desc.yawDegrees = placement.yawDegrees;
		desc.isEnabled = placement.isEnabled;
		desc.isCollisionBox =
			WORLD_PLACEMENT_KIND::COLLISION_BOX == placement.eKind;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_TriggerBox"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_TriggerBoxes"),
			&desc,
			&gameObject)))
		{
			Remove_WorldTriggerBoxes(outEntries);
			m_WorldGameplayStatus =
				"Trigger Box presentation stage failed: " + placement.placementId;
			return false;
		}

		shared_ptr<CTrigger_Box> triggerBox =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == triggerBox)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_TriggerBoxes"),
				gameObject);
			Remove_WorldTriggerBoxes(outEntries);
			m_WorldGameplayStatus =
				"Trigger Box clone type mismatch: " + placement.placementId;
			return false;
		}
		triggerBox->Set_Selected(
			m_SelectedWorldPlacementId == placement.placementId);
		triggerBox->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode);
		outEntries.push_back({ placement.placementId, std::move(triggerBox) });
	}
	return true;
}

void Client::CMapTool::Remove_WorldTriggerBoxes(
	vector<TRIGGER_BOX_ENTRY>& entries)
{
	for (const TRIGGER_BOX_ENTRY& entry : entries)
	{
		if (nullptr != entry.object &&
			m_iAuthoringLevelIndex < ETOUI(LEVEL::END))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_TriggerBoxes"),
				static_pointer_cast<CGameObject>(entry.object));
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_SpawnAnchors"),
				static_pointer_cast<CGameObject>(entry.object));
		}
	}
	entries.clear();
}

bool_t Client::CMapTool::Stage_WorldNpcPreviews(
	const CWorldGameplayDocument& document,
	vector<NPC_PREVIEW_ENTRY>& outEntries,
	const CNavGridPaintDocument* pNavigation,
	const CNavRuntimeBlockerDocument* pBlockers)
{
	outEntries.clear();
	if (m_iAuthoringLevelIndex >= ETOUI(LEVEL::END) ||
		nullptr == m_pDevice || nullptr == m_pContext)
	{
		return false;
	}

	size_t skippedCount = 0;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::NPC != placement.eKind)
			continue;
		std::string navigationStatus;
		if (!Validate_WorldNpcBehaviorNavigation(
				placement,
				nullptr != pNavigation ? *pNavigation : m_NavigationDocument,
				nullptr != pBlockers ? *pBlockers : m_RuntimeBlockerDocument,
				navigationStatus))
		{
			m_WorldGameplayStatus = std::move(navigationStatus);
			Remove_WorldNpcPreviews(outEntries);
			return false;
		}

		const NPC_ACTOR_ENTRY* actor =
			CActorCatalog::Find_Npc(placement.archetypeId);
		const wstring_t modelTag =
			CNpcPresentationAssetService::Get_ModelPrototypeTag(
				placement.archetypeId);
		if (nullptr == actor || modelTag.empty() ||
			FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
				m_pDevice, m_pContext,
				m_iAuthoringLevelIndex, placement.archetypeId)))
		{
			++skippedCount;
			continue;
		}

		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_iAuthoringLevelIndex;
		desc.strModelTag = modelTag;
		desc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		desc.pIdleClip = placement.npcIdleClip.empty() ?
			actor->idleClip.c_str() : placement.npcIdleClip.c_str();
		desc.bSuppressRootMotion = true;
		desc.vPosition = placement.position;
		desc.fYawDegree = placement.yawDegrees;
		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_iAuthoringLevelIndex,
			TEXT("Prototype_GameObject_Npc"),
			m_iAuthoringLevelIndex,
			TEXT("Layer_NpcPreviews"),
			&desc,
			&gameObject)))
		{
			++skippedCount;
			continue;
		}
		shared_ptr<CNpc> npc = dynamic_pointer_cast<CNpc>(gameObject);
		if (nullptr == npc)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_NpcPreviews"),
				gameObject);
			++skippedCount;
			continue;
		}
		const shared_ptr<CModel> model = npc->Get_Model();
		const auto hasClip = [&model](const std::string& clipName)
		{
			if (clipName.empty())
				return true;
			if (nullptr == model)
				return false;
			for (uint32_t index = 0; index < model->Get_NumAnimations(); ++index)
			{
				const char_t* candidate = model->Get_AnimationName(index);
				if (nullptr != candidate && clipName == candidate)
					return true;
			}
			return false;
		};
		const bool_t clipsValid = hasClip(placement.npcIdleClip) &&
			(!placement.npcBehavior.has_value() ||
				(hasClip(placement.npcBehavior->walkClip) &&
				 std::all_of(placement.npcBehavior->actions.begin(),
					 placement.npcBehavior->actions.end(),
					 [&hasClip](const WORLD_NPC_ACTION& action)
					 {
						 return hasClip(action.clipName);
					 })));
		if (!clipsValid)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_NpcPreviews"),
				gameObject);
			++skippedCount;
			continue;
		}
		outEntries.push_back(
			{ placement.placementId, placement.archetypeId, std::move(npc) });
	}
	if (0 < skippedCount)
	{
		m_WorldGameplayStatus =
			"NPC preview stage failed for " + std::to_string(skippedCount) +
			" placement(s): archetype, asset, or authored clip admission failed";
		Remove_WorldNpcPreviews(outEntries);
		return false;
	}
	return true;
}

void Client::CMapTool::Remove_WorldNpcPreviews(
	vector<NPC_PREVIEW_ENTRY>& entries)
{
	for (const NPC_PREVIEW_ENTRY& entry : entries)
	{
		if (nullptr != entry.object &&
			m_iAuthoringLevelIndex < ETOUI(LEVEL::END))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_iAuthoringLevelIndex,
				TEXT("Layer_NpcPreviews"),
				static_pointer_cast<CGameObject>(entry.object));
		}
	}
	entries.clear();
}

void Client::CMapTool::Update_WorldTriggerBoxPresentation(
	const bool_t isVisible)
{
	for (TRIGGER_BOX_ENTRY& entry : m_WorldTriggerBoxes)
	{
		if (nullptr == entry.object)
			continue;
		entry.object->Set_AuthoringVisible(isVisible);
		entry.object->Set_Selected(
			entry.placementId == m_SelectedWorldPlacementId);
	}
	for (TRIGGER_BOX_ENTRY& entry : m_SpawnAnchorBoxes)
	{
		if (nullptr == entry.object)
			continue;
		entry.object->Set_AuthoringVisible(isVisible);
		entry.object->Set_Selected(entry.placementId == m_SelectedSpawnAnchorId);
	}
	/* Destruction outlines follow their own mode instead of the gameplay
	   visibility flag the two loops above share. */
	const std::string pickedId =
		std::to_string(m_iSelectedDeployPlacementId);
	for (TRIGGER_BOX_ENTRY& entry : m_DestructionHighlightBoxes)
	{
		if (nullptr == entry.object)
			continue;
		entry.object->Set_AuthoringVisible(
			m_bOpen && TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode);
		entry.object->Set_Selected(entry.placementId == pickedId);
	}
}
