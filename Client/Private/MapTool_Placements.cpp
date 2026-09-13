#include "imgui.h"
#include "MapTool_Internal.h"
#include "MapAssetObject.h"
#include "WorldSequenceToolPanel.h"
#include "GameInstance.h"
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

namespace
{
	bool_t PreserveLiveDeployPresentation(
		const Client::CDeployPropRuntime& live,
		Client::CDeployPropRuntime& staged)
	{
		for (const auto& entry : staged.Get_Entries())
		{
			const auto previous = live.Find(entry.placement.runtimePlacementId);
			if (nullptr == previous) continue;
			if (nullptr == entry.object ||
				!entry.object->Set_State(previous->Get_State()) ||
				!entry.object->Apply_SurfacePresentation(previous->Get_SurfacePresentation()))
				return false;
		}
		return true;
	}
}



bool_t Client::CMapTool::Try_PickPlacementPosition(float3_t& outPosition) const
{
	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked))
		return false;

	/* Target_PickPos is written by the first depth-tested rendered triangle at
	   the cursor. Do not invent a Y=0 fallback when the view ray misses. */
	outPosition = float3_t(picked.x, picked.y, picked.z);
	return IsFinite(outPosition);
}

bool_t Client::CMapTool::Try_PlaceSelected()
{
	if (!Can_ChangeRuntimeStructure()) return false;
	const MAP_ASSET_ENTRY* pAsset = Get_SelectedAsset();
	if (nullptr == pAsset)
	{
		m_Status = "Select an asset before placing";
		return false;
	}

	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_Status = "No valid surface under the cursor";
		return false;
	}

	const uint64_t placementId = Allocate_EditorPlacementId();
	if (0 == placementId)
	{
		m_Status = "No editor placement ID is available";
		return false;
	}

	MAP_PLACEMENT_RECORD record{};
	record.placementId = placementId;
	record.sourcePlacementId = "editor:" + m_Catalog.Get_AreaId() +
		":" + std::to_string(placementId);
	record.sourceLevel = "EDITOR";
	record.transformSource = "editor";
	record.assetId = pAsset->id;
	record.position = position;
	record.rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	record.signedScale = pAsset->defaultScale;
	record.visible = true;

	PLACED_ENTRY placed{};
	if (!Create_Placement(record, placed))
	{
		m_Status = "Failed to clone map object for " + pAsset->id;
		return false;
	}

	m_iSelectedPlacementId = placementId;
	Authoring_Placements().push_back(std::move(placed));
	Remember_RuntimePlacement(record);
	m_bDirty = true;
	m_Status = "Placed " + pAsset->label +
		"; placement remains armed (Esc cancels).";
	return true;
}

uint64_t Client::CMapTool::Allocate_EditorPlacementId()
{
	for (size_t attempt = 0; attempt <= Authoring_Placements().size(); ++attempt)
	{
		if (0 == m_iNextPlacementId ||
			m_iNextPlacementId > CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
			m_iNextPlacementId = 1;

		const uint64_t candidate = m_iNextPlacementId++;
		if (nullptr == Find_Placement(candidate))
			return candidate;
	}

	return 0;
}

std::wstring Client::CMapTool::Make_LayerTag(
	const std::string& sourceLevel) const
{
	return CMapPlacementRuntime::Make_LayerTag(sourceLevel);
}

bool_t Client::CMapTool::Create_Placement(
	const MAP_PLACEMENT_RECORD& record, PLACED_ENTRY& outEntry)
{
	return CMapPlacementRuntime::Create_Placement(
		m_iAuthoringLevelIndex, m_Catalog, record, outEntry);
}

bool_t Client::CMapTool::Stage_PlacementRuntime(
	const vector<MAP_PLACEMENT_RECORD>& records,
	vector<PLACED_ENTRY>& outPlacements,
	vector<STATIC_BATCH_ENTRY>& outBatches)
{
	return CMapPlacementRuntime::Stage_PlacementRuntime(
		m_iAuthoringLevelIndex,
		m_Catalog,
		records,
		outPlacements,
		outBatches);
}

void Client::CMapTool::Remove_PlacementRuntime(
	vector<PLACED_ENTRY>& placements,
	vector<STATIC_BATCH_ENTRY>& batches)
{
	CMapPlacementRuntime::Remove_PlacementRuntime(
		m_iAuthoringLevelIndex, placements, batches);
}

bool_t Client::CMapTool::Set_RuntimeVisible(
	PLACED_ENTRY& entry, bool_t visible)
{
	return CMapPlacementRuntime::Set_RuntimeVisible(entry, visible);
}

bool_t Client::CMapTool::Remove_Placement(uint64_t placementId)
{
	if (!Can_ChangeRuntimeStructure()) return false;
	const auto iter = std::find_if(Authoring_Placements().begin(), Authoring_Placements().end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	if (iter == Authoring_Placements().end())
		return false;

	if (nullptr != iter->object)
	{
		if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iAuthoringLevelIndex, iter->layerTag,
			static_pointer_cast<CGameObject>(iter->object))))
			return false;
	}
	else if (nullptr != iter->batch)
	{
		if (FAILED(iter->batch->Set_InstanceVisible(placementId, false)))
			return false;
	}
	else
		return false;

	Authoring_Placements().erase(iter);
	Forget_RuntimePlacement(placementId);
	if (m_iSelectedPlacementId == placementId)
		m_iSelectedPlacementId = 0;
	m_bDirty = true;
	return true;
}

void Client::CMapTool::Remove_AllPlacements()
{
	if (!Can_ChangeRuntimeStructure()) return;
	Remove_PlacementRuntime(Authoring_Placements(), Authoring_Batches());
	if (m_bRuntimeAuthoring) Reset_RuntimePlacementDraft({});
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	m_bDirty = true;
}

bool_t Client::CMapTool::Save_Placements(
	const bool_t linkedTransactionAlreadyLocked,
	vector<MAP_PLACEMENT_RECORD>* outSavedRecords)
{
	if (!linkedTransactionAlreadyLocked &&
		nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready())
	{
		return Save_PlacementsAndWorldSequences();
	}
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	std::string linkedStatus;
	if (!linkedTransactionAlreadyLocked)
	{
		const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
		const std::filesystem::path placementPath = nullptr != active ?
			active->sourcePlacements : std::filesystem::path{};
		const std::filesystem::path sequencePath = Get_WorldSequencePath();
		if (!authoringLock.Acquire(sequencePath, linkedStatus) ||
			!RecoverAuthoringTransactionUnderLock(
				placementPath, sequencePath, linkedStatus))
		{
			m_Status = linkedStatus;
			return false;
		}
	}
	vector<MAP_PLACEMENT_RECORD> document;
	document.reserve(Authoring_Placements().size());
	for (const PLACED_ENTRY& entry : Authoring_Placements())
	{
		const bool_t hasObject = nullptr != entry.object;
		const bool_t hasBatch = nullptr != entry.batch;
		if (hasObject == hasBatch ||
			nullptr == m_Catalog.Find(entry.record.assetId))
		{
			m_Status = "Save aborted: runtime representation is invalid";
			return false;
		}

		MAP_PLACEMENT_RECORD stored = Authored_Placement(entry);
		if (entry.record.sourceLevel.starts_with("VALTAN_PHASE_"))
			stored.visible = false;
		if (!CMapPlacementDocument::Is_Valid(stored, m_Catalog))
		{
			m_Status = "Save aborted: a transform is invalid";
			return false;
		}
		document.push_back(std::move(stored));
	}

	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	const std::filesystem::path authoringPath = nullptr != active ?
		active->sourcePlacements : std::filesystem::path{};
	vector<MAP_PLACEMENT_RECORD> storedDocument;
	if (authoringPath.empty() ||
		!CMapPlacementDocument::Write(
		authoringPath, m_Catalog.Get_AreaId(),
		document, m_Catalog, m_Status, &storedDocument))
		return false;
	document = std::move(storedDocument);

	m_bDirty = false;
	m_Status += " (authoring only; publish step required)";
	if (nullptr != outSavedRecords)
		*outSavedRecords = document;
	return true;
}

bool_t Client::CMapTool::Save_PlacementsAndWorldSequences()
{
	if (nullptr == m_pWorldSequenceToolPanel ||
		!m_pWorldSequenceToolPanel->Is_Ready())
	{
		m_Status = "Linked map and sequence save is unavailable";
		return false;
	}
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	const std::filesystem::path placementPath = nullptr != active ?
		active->sourcePlacements : std::filesystem::path{};
	const std::filesystem::path sequencePath = Get_WorldSequencePath();
	AUTHORING_FILE_BACKUP placementBackup;
	AUTHORING_FILE_BACKUP sequenceBackup;
	std::string transactionStatus;
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(sequencePath, transactionStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		placementPath, sequencePath, transactionStatus))
	{
		m_Status = transactionStatus;
		return false;
	}
	if (!m_pWorldSequenceToolPanel->Matches_LinkedSourceBaseline(
		transactionStatus))
	{
		m_Status = transactionStatus + ". Reload before saving.";
		return false;
	}
	if (!PrepareAuthoringBackup(
		placementPath, placementBackup, transactionStatus))
	{
		m_Status = transactionStatus;
		return false;
	}
	if (!PrepareAuthoringBackup(
		sequencePath, sequenceBackup, transactionStatus))
	{
		DiscardAuthoringBackup(placementBackup);
		DiscardAuthoringBackup(sequenceBackup);
		m_Status = transactionStatus;
		return false;
	}
	if (!WriteAuthoringTransactionMarker(
		placementBackup, sequenceBackup, transactionStatus))
	{
		DiscardAuthoringBackup(placementBackup);
		DiscardAuthoringBackup(sequenceBackup);
		m_Status = transactionStatus;
		return false;
	}

	const bool_t mapWasDirty = m_bDirty;
	const bool_t sequenceWasDirty = m_pWorldSequenceToolPanel->Is_Dirty();
	vector<MAP_PLACEMENT_RECORD> intendedPlacements;
	CWorldSequenceToolPanel verification;
	bool_t verificationReady = false;
	bool_t saved = Save_Placements(true, &intendedPlacements);
	if (saved)
	{
		saved = m_pWorldSequenceToolPanel->Save(
			m_Catalog, Authoring_Placements(), Authoring_Deploy(), transactionStatus);
		if (!saved)
			m_Status = transactionStatus;
	}
	if (saved)
	{
		std::vector<MAP_PLACEMENT_RECORD> verifiedPlacements;
		if (!CMapPlacementDocument::Read(
			placementPath, m_Catalog, verifiedPlacements, transactionStatus))
		{
			saved = false;
			m_Status = "Linked save verification failed for Map Placements: " +
				transactionStatus;
		}
		else if (!AreExactlySamePlacementRecords(
			intendedPlacements, verifiedPlacements))
		{
			saved = false;
			m_Status =
				"Linked save verification found different Map Placement content";
		}
		else
		{
			if (!verification.Load_Area(sequencePath, placementPath,
				m_Catalog.Get_AreaId(), m_Catalog, verifiedPlacements,
				Authoring_Deploy(), transactionStatus))
			{
				saved = false;
				m_Status = "Linked save verification failed for World Sequences: " +
					transactionStatus;
			}
			else
			{
				std::vector<MAP_PLACEMENT_RECORD> stableVerifiedPlacements;
				if (!CMapPlacementDocument::Read(placementPath, m_Catalog,
					stableVerifiedPlacements, transactionStatus) ||
					!AreExactlySamePlacementRecords(
						verifiedPlacements, stableVerifiedPlacements) ||
					!verification.Matches_LinkedSourceBaseline(transactionStatus))
				{
					saved = false;
					m_Status =
						"Linked save verification source changed during final read: " +
						transactionStatus;
				}
				else if (!m_pWorldSequenceToolPanel->Has_SameDocument(verification))
				{
					saved = false;
					m_Status =
						"Linked save verification found different World Sequence content";
				}
				else
				{
					verificationReady = true;
				}
			}
		}
	}
	if (saved && (!verificationReady ||
		!verification.Matches_LinkedSourceBaseline(transactionStatus)))
	{
		saved = false;
		m_Status = "Linked save source changed before commit: " +
			transactionStatus;
	}
	if (!saved)
	{
		const std::string saveFailure = m_Status;
		const bool_t mapRestored = RestoreAuthoringBackup(placementBackup);
		const bool_t sequenceRestored = RestoreAuthoringBackup(sequenceBackup);
		m_bDirty = mapWasDirty;
		if (sequenceWasDirty)
			m_pWorldSequenceToolPanel->Restore_DirtyAfterFailedTransaction();
		if (mapRestored && sequenceRestored &&
			ClearAuthoringTransactionMarker(sequencePath))
		{
			DiscardAuthoringBackup(placementBackup);
			DiscardAuthoringBackup(sequenceBackup);
			m_Status = "Linked map/sequence save rolled back: " + saveFailure;
		}
		else
		{
			m_Status = "CRITICAL: linked save recovery is locked; preserve " +
				placementBackup.backup.filename().string() + " and " +
				sequenceBackup.backup.filename().string();
		}
		return false;
	}
	if (!ClearAuthoringTransactionMarker(sequencePath))
	{
		const bool_t mapRestored = RestoreAuthoringBackup(placementBackup);
		const bool_t sequenceRestored = RestoreAuthoringBackup(sequenceBackup);
		m_bDirty = mapWasDirty;
		if (sequenceWasDirty)
			m_pWorldSequenceToolPanel->Restore_DirtyAfterFailedTransaction();
		m_Status = mapRestored && sequenceRestored ?
			"Linked save marker cleanup failed; original files were restored, the "
			"editor baseline stayed unchanged, and the pending marker was preserved" :
			"CRITICAL: marker cleanup rollback failed; pending "
			"marker and backups were preserved";
		return false;
	}
	m_pWorldSequenceToolPanel->Adopt_VerifiedLinkedSourceBaseline(verification);
	DiscardAuthoringBackup(placementBackup);
	DiscardAuthoringBackup(sequenceBackup);
	m_Status = "Saved linked Map Placements and World Sequences";
	return true;
}

bool_t Client::CMapTool::Load_Placements()
{
	if (!Can_ChangeRuntimeStructure()) return false;
	if (!m_Catalog.Is_Ready())
		return false;

	vector<MAP_PLACEMENT_RECORD> document;
	std::string loadStatus;
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	const std::filesystem::path authoringPath = nullptr != active ?
		active->sourcePlacements : std::filesystem::path{};
	const std::filesystem::path sequencePath = Get_WorldSequencePath();
	SCOPED_AUTHORING_SAVE_LOCK authoringLock;
	if (!authoringLock.Acquire(sequencePath, loadStatus) ||
		!RecoverAuthoringTransactionUnderLock(
		authoringPath, sequencePath, loadStatus))
	{
		m_Status = loadStatus;
		return false;
	}
	std::error_code authoringError;
	const bool_t hasAuthoring =
		!authoringPath.empty() &&
		std::filesystem::is_regular_file(
			authoringPath, authoringError);
	if (authoringError || !hasAuthoring)
	{
		m_Status = authoringError ?
			"Could not inspect map placement authoring source" :
			"Map placement authoring source is missing";
		return false;
	}
	if (!CMapPlacementDocument::Read(
		authoringPath, m_Catalog, document, loadStatus))
	{
		m_Status = loadStatus;
		return false;
	}
	auto stagedWorldSequencePanel =
		std::make_unique<CWorldSequenceToolPanel>();
	if (!stagedWorldSequencePanel->Load_Area(
		Get_WorldSequencePath(), authoringPath, m_Catalog.Get_AreaId(),
		m_Catalog, document, Authoring_Deploy(), loadStatus))
	{
		m_Status = loadStatus;
		return false;
	}
	std::vector<MAP_PLACEMENT_RECORD> stableLinkedDocument;
	if (!CMapPlacementDocument::Read(authoringPath, m_Catalog,
		stableLinkedDocument, loadStatus) ||
		!AreExactlySamePlacementRecords(document, stableLinkedDocument) ||
		!stagedWorldSequencePanel->Matches_LinkedSourceBaseline(loadStatus))
	{
		m_Status = "Linked map/sequence source changed while Reload was staging: " +
			loadStatus;
		return false;
	}

	vector<PLACED_ENTRY> stagedPlacements;
	vector<STATIC_BATCH_ENTRY> stagedBatches;
	if (!Stage_PlacementRuntime(
		document, stagedPlacements, stagedBatches))
	{
		Remove_PlacementRuntime(stagedPlacements, stagedBatches);
		m_Status = "Map runtime staging rolled back";
		return false;
	}
	if (nullptr != m_pWorldSequenceToolPanel)
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, Authoring_Placements(),
			Authoring_Deploy());
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			Remove_PlacementRuntime(stagedPlacements, stagedBatches);
			m_Status = m_pWorldSequenceToolPanel->Get_Status();
			return false;
		}
	}

	if (m_bRuntimeAuthoring)
	{
		std::unordered_map<uint64_t, bool_t> liveVisibility;
		for (const auto& entry : Authoring_Placements())
		{
			bool_t visible = false;
			if (!CMapPlacementRuntime::Try_GetRuntimeVisible(entry, visible))
			{
				Remove_PlacementRuntime(stagedPlacements, stagedBatches);
				m_Status = "Could not preserve live map visibility; Reload cancelled";
				return false;
			}
			// Preserve only a runtime override, not an unsaved inspector edit.
			if (visible != Authored_Placement(entry).visible)
				liveVisibility.emplace(entry.record.placementId, visible);
		}
		for (auto& entry : stagedPlacements)
		{
			const auto previous = liveVisibility.find(entry.record.placementId);
			if (previous != liveVisibility.end() && !Set_RuntimeVisible(entry, previous->second))
			{
				Remove_PlacementRuntime(stagedPlacements, stagedBatches);
				m_Status = "Could not restore live map visibility; Reload cancelled";
				return false;
			}
		}
	}
	Remove_PlacementRuntime(Authoring_Placements(), Authoring_Batches());
	Authoring_Placements() = std::move(stagedPlacements);
	Authoring_Batches() = std::move(stagedBatches);
	if (m_bRuntimeAuthoring) Reset_RuntimePlacementDraft(document);
	m_pWorldSequenceToolPanel = std::move(stagedWorldSequencePanel);
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : Authoring_Placements())
	{
		if ((entry.record.transformSource == "editor" ||
			entry.record.transformSource == "legacy") &&
			entry.record.placementId <=
			CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
		{
			m_iNextPlacementId = (std::max)(
				m_iNextPlacementId, entry.record.placementId + 1);
		}
	}
	m_bDirty = false;
	if (!m_bRuntimeAuthoring) Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
	const size_t fallbackCount = static_cast<size_t>(std::count_if(
		Authoring_Placements().begin(), Authoring_Placements().end(),
		[](const PLACED_ENTRY& entry)
		{
			return nullptr != entry.object;
		}));
	m_Status = "Loaded " + std::to_string(Authoring_Placements().size()) +
		" placements / " + std::to_string(Authoring_Batches().size()) +
		" batches / " + std::to_string(fallbackCount) + " fallbacks";
	return true;
}

bool_t Client::CMapTool::Load_DeployProps()
{
	if (!Can_ChangeRuntimeStructure()) return false;
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor)
	{
		m_Status = "DeployProp reload requires an active editor Area";
		return false;
	}

	CDeployPropRuntime stagedRuntime;
	if (!Stage_DeployProps(*descriptor, stagedRuntime))
		return false;
	if (m_bRuntimeAuthoring && !PreserveLiveDeployPresentation(Authoring_Deploy(), stagedRuntime))
	{
		m_Status = "Could not preserve live Deploy presentation; Reload cancelled";
		return false;
	}
	Authoring_Deploy() = std::move(stagedRuntime);
	m_DeployPhase = DEPLOY_PROP_STATE::INTACT;
	m_bDeployDirty = false;
	m_bAnimatedPropPlacementArmed = false;
	m_iSelectedAnimatedPropPlacementId = 0u;
	Sync_AnimatedPropTransformDraft();
	m_Status = "Loaded " + std::to_string(Authoring_Placements().size()) +
		" map placements + " +
		std::to_string(Authoring_Deploy().Get_Entries().size()) +
		" gameplay DeployProps";
	return true;
}

bool_t Client::CMapTool::Stage_DeployProps(
	const EDITOR_AREA_DESCRIPTOR& descriptor,
	CDeployPropRuntime& outRuntime)
{
	if (descriptor.sourceDeployCatalog.empty() &&
		descriptor.sourceDeployPlacements.empty())
	{
		return true;
	}
	if (descriptor.sourceDeployCatalog.empty() ||
		descriptor.sourceDeployPlacements.empty())
	{
		m_Status = "DeployProp authoring file pair is incomplete: " +
			descriptor.areaId;
		return false;
	}

	std::error_code catalogError;
	std::error_code placementError;
	const bool_t catalogExists = std::filesystem::is_regular_file(
		descriptor.sourceDeployCatalog, catalogError);
	const bool_t placementExists = std::filesystem::is_regular_file(
		descriptor.sourceDeployPlacements, placementError);
	if (catalogError || placementError || !catalogExists || !placementExists)
	{
		m_Status = "DeployProp authoring source pair is unavailable: " +
			descriptor.areaId;
		return false;
	}

	CDeployPropCatalog catalog;
	if (!catalog.Load(
		descriptor.sourceDeployCatalog,
		descriptor.sourceDeployPlacements,
		descriptor.areaId))
	{
		m_Status = catalog.Get_Status();
		return false;
	}
	if (!Ensure_DeployAuthoringPrototypes(catalog))
		return false;
	if (!outRuntime.Load(m_iAuthoringLevelIndex, std::move(catalog)))
	{
		m_Status = outRuntime.Get_Status();
		return false;
	}
	return true;
}

bool_t Client::CMapTool::Commit_DeployCatalog(
	CDeployPropCatalog catalog,
	const std::string& successStatus)
{
	if (!Can_ChangeRuntimeStructure()) return false;
	if (!catalog.Is_Ready() || m_iAuthoringLevelIndex >= ETOUI(LEVEL::END))
	{
		m_Status = "DeployProp catalog commit is unavailable";
		return false;
	}

	/* Sequence preview and destruction preview both own live seams into the
	   Deploy objects this commit replaces. Release them before the runtime
	   that owns those objects is rebuilt, and refuse the commit when a
	   preview cannot be restored. */
	if (nullptr != m_pWorldSequenceToolPanel && m_Catalog.Is_Ready())
	{
		m_pWorldSequenceToolPanel->Stop_AndRestore(
			m_iAuthoringLevelIndex, m_Catalog, Authoring_Placements(), Authoring_Deploy());
		if (m_pWorldSequenceToolPanel->Is_PreviewActive())
		{
			m_Status = m_pWorldSequenceToolPanel->Get_Status();
			return false;
		}
	}
	Restore_DestructionPreview();
	if (nullptr != m_pDestructionSimulationController)
		m_pDestructionSimulationController->Clear();

	if (!Ensure_DeployAuthoringPrototypes(catalog))
		return false;

	CDeployPropRuntime stagedRuntime;
	if (!stagedRuntime.Load(m_iAuthoringLevelIndex, std::move(catalog)))
	{
		m_Status = stagedRuntime.Get_Status();
		return false;
	}

	if (m_bRuntimeAuthoring && !PreserveLiveDeployPresentation(Authoring_Deploy(), stagedRuntime))
	{
		m_Status = "Could not preserve live Deploy presentation; edit cancelled";
		return false;
	}
	Authoring_Deploy() = std::move(stagedRuntime);
	if (!m_bRuntimeAuthoring && DEPLOY_PROP_STATE::INTACT != m_DeployPhase &&
		!Authoring_Deploy().Set_State_All(m_DeployPhase))
	{
		m_DeployPhase = DEPLOY_PROP_STATE::INTACT;
	}
	m_bDeployDirty = true;
	m_Status = successStatus;
	return true;
}

bool_t Client::CMapTool::Save_DeployPlacements()
{
	const EDITOR_AREA_DESCRIPTOR* active = Get_ActiveEditorArea();
	if (nullptr == active || active->sourceDeployCatalog.empty() ||
		active->sourceDeployPlacements.empty())
	{
		m_Status = "This Area declares no Deploy placement authoring document";
		return false;
	}
	if (!Authoring_Deploy().Get_Catalog().Is_Ready())
	{
		m_Status = "DeployProp catalog is not loaded";
		return false;
	}

	CDeployPropCatalog saved = Authoring_Deploy().Get_Catalog();
	if (!saved.Save_Placements(active->sourceDeployPlacements))
	{
		m_Status = saved.Get_Status();
		return false;
	}

	/* The replaced file is the only input the next load reads, so read it
	   back and require the exact rows this tool intended before clearing
	   the dirty flag. */
	CDeployPropCatalog verification;
	if (!verification.Load(
		active->sourceDeployCatalog,
		active->sourceDeployPlacements,
		active->areaId))
	{
		m_Status = "DeployProp placement save verification failed: " +
			verification.Get_Status();
		return false;
	}
	const std::vector<DEPLOY_PROP_PLACEMENT>& intended = saved.Get_Placements();
	const std::vector<DEPLOY_PROP_PLACEMENT>& stored =
		verification.Get_Placements();
	bool_t identical = intended.size() == stored.size();
	for (size_t index = 0; identical && index < intended.size(); ++index)
	{
		const DEPLOY_PROP_PLACEMENT& left = intended[index];
		const DEPLOY_PROP_PLACEMENT& right = stored[index];
		identical =
			left.runtimePlacementId == right.runtimePlacementId &&
			left.deployActorId == right.deployActorId &&
			left.propDefinitionId == right.propDefinitionId &&
			left.sourcePlacementId == right.sourcePlacementId &&
			left.assetId == right.assetId &&
			left.position.x == right.position.x &&
			left.position.y == right.position.y &&
			left.position.z == right.position.z &&
			left.rotationQuaternion.x == right.rotationQuaternion.x &&
			left.rotationQuaternion.y == right.rotationQuaternion.y &&
			left.rotationQuaternion.z == right.rotationQuaternion.z &&
			left.rotationQuaternion.w == right.rotationQuaternion.w &&
			left.uniformScale == right.uniformScale &&
			left.destructible == right.destructible &&
			left.stateOffActionId == right.stateOffActionId &&
			left.triggerBinaryOccurrenceCount ==
				right.triggerBinaryOccurrenceCount &&
			left.provenance == right.provenance;
	}
	if (!identical)
	{
		m_Status =
			"DeployProp placement save verification found different content";
		return false;
	}

	m_bDeployDirty = false;
	m_Status = "Saved " + std::to_string(stored.size()) +
		" Deploy placements (authoring only; publish step required)";
	return true;
}

bool_t Client::CMapTool::Try_PlaceSelectedDeploy()
{
	const DEPLOY_PROP_ASSET_ENTRY* asset = Get_SelectedDeployAsset();
	if (nullptr == asset)
	{
		m_Status = "Select an animated prop asset before placing";
		return false;
	}
	if (DEPLOY_PROP_MODEL_KIND::ANIM != asset->kind)
	{
		m_Status = "Only cooked ANIM Deploy assets are placed by this tool";
		return false;
	}

	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_Status = "No valid surface under the cursor";
		return false;
	}

	const uint64_t placementId = Allocate_AnimatedPropPlacementId();
	if (0u == placementId)
	{
		m_Status = "No editor Deploy placement ID is available";
		return false;
	}

	DEPLOY_PROP_PLACEMENT placement{};
	placement.runtimePlacementId = placementId;
	placement.sourcePlacementId = "editor:" +
		Authoring_Deploy().Get_Catalog().Get_AreaId() + ":" +
		std::to_string(placementId);
	placement.assetId = asset->id;
	placement.position = position;
	placement.rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	placement.uniformScale = 1.f;
	placement.provenance =
		DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED;

	const std::string label = asset->label;
	CDeployPropCatalog staged = Authoring_Deploy().Get_Catalog();
	if (!staged.Add_ProjectAuthoredPlacement(placement))
	{
		m_Status = staged.Get_Status();
		return false;
	}
	if (!Commit_DeployCatalog(std::move(staged),
		"Placed " + label + " as animated prop #" +
		std::to_string(placementId) +
		"; placement remains armed (Esc cancels)."))
	{
		return false;
	}

	m_iSelectedAnimatedPropPlacementId = placementId;
	Sync_AnimatedPropTransformDraft();
	return true;
}

bool_t Client::CMapTool::Apply_AnimatedPropTransform()
{
	const DEPLOY_RUNTIME_ENTRY* selected = Get_SelectedAnimatedProp();
	if (nullptr == selected)
	{
		m_Status = "Select a placed animated prop first";
		return false;
	}
	if (DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED !=
		selected->placement.provenance)
	{
		m_Status = "Source-extracted Deploy placements are read-only";
		return false;
	}
	if (m_iAnimatedPropDraftPlacementId !=
		selected->placement.runtimePlacementId)
	{
		m_Status = "The transform draft does not belong to the selection";
		return false;
	}
	if (!IsFinite(m_AnimatedPropDraftPosition) ||
		!IsFiniteQuaternion(m_AnimatedPropDraftRotation) ||
		!std::isfinite(m_fAnimatedPropDraftScale) ||
		m_fAnimatedPropDraftScale <= 0.000001f)
	{
		m_Status = "The transform draft is not a valid placement transform";
		return false;
	}

	DEPLOY_PROP_PLACEMENT updated = selected->placement;
	updated.position = m_AnimatedPropDraftPosition;
	updated.rotationQuaternion = m_AnimatedPropDraftRotation;
	updated.uniformScale = m_fAnimatedPropDraftScale;

	CDeployPropCatalog staged = Authoring_Deploy().Get_Catalog();
	if (!staged.Update_ProjectAuthoredPlacement(updated))
	{
		m_Status = staged.Get_Status();
		return false;
	}
	if (!Commit_DeployCatalog(std::move(staged),
		"Applied the transform of animated prop #" +
		std::to_string(updated.runtimePlacementId)))
	{
		return false;
	}

	Sync_AnimatedPropTransformDraft();
	return true;
}

bool_t Client::CMapTool::Remove_SelectedAnimatedProp()
{
	const DEPLOY_RUNTIME_ENTRY* selected = Get_SelectedAnimatedProp();
	if (nullptr == selected)
	{
		m_Status = "Select a placed animated prop first";
		return false;
	}
	if (DEPLOY_PROP_PLACEMENT_PROVENANCE::PROJECT_AUTHORED !=
		selected->placement.provenance)
	{
		m_Status = "Source-extracted Deploy placements cannot be removed";
		return false;
	}

	const uint64_t placementId = selected->placement.runtimePlacementId;
	const bool_t wasDirty = m_bDeployDirty;
	CDeployPropCatalog restore = Authoring_Deploy().Get_Catalog();
	CDeployPropCatalog staged = restore;
	if (!staged.Remove_ProjectAuthoredPlacement(placementId))
	{
		m_Status = staged.Get_Status();
		return false;
	}
	if (!Commit_DeployCatalog(std::move(staged),
		"Removed animated prop #" + std::to_string(placementId)))
	{
		return false;
	}

	/* A world sequence animation track may still bind the removed placement.
	   The sequence document owns that reference rule, so ask its validator
	   and put the Deploy runtime back when the removal orphans a slot. */
	std::string sequenceStatus;
	if (nullptr != m_pWorldSequenceToolPanel &&
		m_pWorldSequenceToolPanel->Is_Ready() &&
		!m_pWorldSequenceToolPanel->Validate(
			m_Catalog, Authoring_Placements(), Authoring_Deploy(), sequenceStatus))
	{
		const std::string failure =
			"Animated prop #" + std::to_string(placementId) +
			" is still used by a world sequence: " + sequenceStatus;
		if (!Commit_DeployCatalog(std::move(restore),
			"Restored animated prop #" + std::to_string(placementId)))
		{
			return false;
		}
		m_bDeployDirty = wasDirty;
		m_Status = failure;
		return false;
	}

	m_iSelectedAnimatedPropPlacementId = 0u;
	Sync_AnimatedPropTransformDraft();
	return true;
}

uint64_t Client::CMapTool::Allocate_AnimatedPropPlacementId() const
{
	/* Deploy placements share the map placement ID domain rule: an extracted
	   SOURCE_EXACT row keeps the high bit set, so a project-authored row must
	   stay at or below MAX_EDITOR_PLACEMENT_ID and can never impersonate one. */
	uint64_t candidate = 1u;
	for (const DEPLOY_PROP_PLACEMENT& row :
		Authoring_Deploy().Get_Catalog().Get_Placements())
	{
		if (row.runtimePlacementId >
			CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
		{
			continue;
		}
		if (row.runtimePlacementId >= candidate)
			candidate = row.runtimePlacementId + 1u;
	}
	return candidate > CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID ?
		0u : candidate;
}

const Client::DEPLOY_PROP_ASSET_ENTRY*
Client::CMapTool::Get_SelectedDeployAsset() const
{
	if (m_SelectedDeployAssetId.empty())
		return nullptr;
	return Authoring_Deploy().Get_Catalog().Find(m_SelectedDeployAssetId);
}

const Client::DEPLOY_RUNTIME_ENTRY*
Client::CMapTool::Get_SelectedAnimatedProp() const
{
	if (0u == m_iSelectedAnimatedPropPlacementId)
		return nullptr;
	const std::vector<DEPLOY_RUNTIME_ENTRY>& entries =
		Authoring_Deploy().Get_Entries();
	const auto iter = std::find_if(entries.begin(), entries.end(),
		[this](const DEPLOY_RUNTIME_ENTRY& entry)
		{
			return entry.placement.runtimePlacementId ==
				m_iSelectedAnimatedPropPlacementId;
		});
	return iter == entries.end() ? nullptr : &*iter;
}

void Client::CMapTool::Sync_AnimatedPropTransformDraft()
{
	const DEPLOY_RUNTIME_ENTRY* selected = Get_SelectedAnimatedProp();
	if (nullptr == selected)
	{
		m_iAnimatedPropDraftPlacementId = 0u;
		m_AnimatedPropDraftPosition = {};
		m_AnimatedPropDraftRotation = float4_t(0.f, 0.f, 0.f, 1.f);
		m_fAnimatedPropDraftScale = 1.f;
		return;
	}
	m_iAnimatedPropDraftPlacementId = selected->placement.runtimePlacementId;
	m_AnimatedPropDraftPosition = selected->placement.position;
	m_AnimatedPropDraftRotation = selected->placement.rotationQuaternion;
	m_fAnimatedPropDraftScale = selected->placement.uniformScale;
}

void Client::CMapTool::Remove_DeployProps()
{
	Authoring_Deploy().Clear();
}

void Client::CMapTool::Set_DeployPhase(DEPLOY_PROP_STATE state)
{
	if (!Authoring_Deploy().Set_State_All(state))
	{
		m_Status = Authoring_Deploy().Get_Status();
		return;
	}
	m_DeployPhase = state;

	const bool_t arenaDestroyed =
		DEPLOY_PROP_STATE::INTACT != state;
	if (!Set_NavigationCondition(
		"VALTAN_ARENA_DESTROYED",
		arenaDestroyed) &&
		0 != m_RuntimeBlockerDocument.Get_RegionCount())
	{
		m_NavigationStatus =
			"Deploy visual state changed, but Nav blocker condition failed";
	}
}

void Client::CMapTool::Set_EnvironmentPhase(ENVIRONMENT_PHASE phase)
{
	m_EnvironmentPhase = phase;
	for (PLACED_ENTRY& entry : Authoring_Placements())
	{
		bool_t visible = entry.record.visible;
		if (entry.record.sourceLevel == "VALTAN_PHASE_SPACEHOLE")
			visible = phase != ENVIRONMENT_PHASE::BASELINE;
		else if (entry.record.sourceLevel == "VALTAN_PHASE_CHAOSGATE")
			visible = phase == ENVIRONMENT_PHASE::CHAOS_GATE;
		if (!m_bShowBernLandscape &&
			IsBernLandscapePlacement(m_Catalog, entry.record))
		{
			visible = false;
		}
		Set_RuntimeVisible(entry, visible);
	}
}

bool_t Client::CMapTool::Try_PickDeployProp(
	uint64_t& outRuntimePlacementId,
	std::string& outFailure) const
{
	outFailure.clear();
	if (!Authoring_Deploy().Is_Loaded() ||
		Authoring_Deploy().Get_Entries().empty())
	{
		outFailure = "Wall pick unavailable: DeployProp runtime is not ready";
		return false;
	}

	float3_t picked{};
	if (!Try_PickPlacementPosition(picked))
	{
		outFailure =
			"Wall pick missed: no rendered surface under the cursor";
		return false;
	}

	/* The depth pick returns the surface point of whatever was drawn, so the
	   owning prop is the one whose world bounds contain that point. Ties go to
	   the nearest centre so overlapping props stay selectable. */
	constexpr f32_t tolerance = 0.05f;
	uint64_t bestId = 0u;
	f32_t bestDistance = 0.f;
	size_t missingBounds = 0u;
	for (const DEPLOY_RUNTIME_ENTRY& entry : Authoring_Deploy().Get_Entries())
	{
		float3_t center{};
		float3_t halfExtents{};
		if (nullptr == entry.object ||
			!entry.object->Get_WorldBounds(center, halfExtents))
		{
			++missingBounds;
			continue;
		}
		if (
			std::abs(picked.x - center.x) > halfExtents.x + tolerance ||
			std::abs(picked.y - center.y) > halfExtents.y + tolerance ||
			std::abs(picked.z - center.z) > halfExtents.z + tolerance)
		{
			continue;
		}
		const f32_t offsetX = picked.x - center.x;
		const f32_t offsetY = picked.y - center.y;
		const f32_t offsetZ = picked.z - center.z;
		const f32_t distance =
			offsetX * offsetX + offsetY * offsetY + offsetZ * offsetZ;
		if (0u == bestId || distance < bestDistance)
		{
			bestId = entry.placement.runtimePlacementId;
			bestDistance = distance;
		}
	}
	if (0u == bestId)
	{
		outFailure =
			"Wall pick missed: the rendered surface is not a loaded DeployProp";
		if (0u != missingBounds)
			outFailure += " (" + std::to_string(missingBounds) +
				" props have no model bounds)";
		return false;
	}
	outRuntimePlacementId = bestId;
	return true;
}

Client::CMapTool::PLACED_ENTRY* Client::CMapTool::Find_Placement(uint64_t placementId)
{
	const auto iter = std::find_if(Authoring_Placements().begin(), Authoring_Placements().end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	return iter == Authoring_Placements().end() ? nullptr : &*iter;
}
