#include "MapPlacementEditSession.h"

#ifdef _DEBUG
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapAuthoringHost.h"
#include "MapStaticBatchObject.h"
#include "Trigger_Box.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <new>
#include <type_traits>
#include <iterator>
#include <system_error>
#include <unordered_set>

namespace
{
	/* Byte exact reads and writes of the placement document, so the save
	   baseline, the rollback copy and the freshness check agree to the byte
	   with what CMapPlacementDocument wrote. Same contract as the Map Tool's
	   own helpers; a second tool may not use a looser one. */
	bool_t ReadDocumentBytes(
		const std::filesystem::path& path,
		std::string& outBytes)
	{
		std::error_code error;
		const uintmax_t size = std::filesystem::file_size(path, error);
		if (error || size > 64u * 1024u * 1024u)
			return false;
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return false;
		outBytes.assign(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
		return !input.bad() && outBytes.size() == size;
	}

	bool_t WriteDocumentBytes(
		const std::filesystem::path& path,
		const std::string& bytes)
	{
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		if (!output ||
			!output.write(bytes.data(), static_cast<std::streamsize>(bytes.size())) ||
			!output.flush())
		{
			return false;
		}
		output.close();
		std::string reopened;
		return ReadDocumentBytes(path, reopened) && reopened == bytes;
	}

	/* Same temporary then rename commit the document writer uses. */
	bool_t WriteDocumentBytesAtomic(
		const std::filesystem::path& path,
		const std::string& bytes)
	{
		std::filesystem::path temporary = path;
		temporary += L".worldlevel.restore.tmp";
		if (!WriteDocumentBytes(temporary, bytes))
			return false;
		if (!MoveFileExW(temporary.c_str(), path.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			std::error_code removeError;
			std::filesystem::remove(temporary, removeError);
			return false;
		}
		return true;
	}

	bool_t SamePose(
		const Client::MAP_PLACEMENT_RECORD& left,
		const Client::MAP_PLACEMENT_RECORD& right)
	{
		return left.position.x == right.position.x &&
			left.position.y == right.position.y &&
			left.position.z == right.position.z &&
			left.rotationQuaternion.x == right.rotationQuaternion.x &&
			left.rotationQuaternion.y == right.rotationQuaternion.y &&
			left.rotationQuaternion.z == right.rotationQuaternion.z &&
			left.rotationQuaternion.w == right.rotationQuaternion.w &&
			left.signedScale.x == right.signedScale.x &&
			left.signedScale.y == right.signedScale.y &&
			left.signedScale.z == right.signedScale.z;
	}

	bool_t IsMirrored(const float3_t& scale)
	{
		return scale.x * scale.y * scale.z < 0.f;
	}
}

Client::CMapPlacementEditSession::~CMapPlacementEditSession()
{
	Remove_Outline();
}

bool_t Client::CMapPlacementEditSession::Bind(
	const BIND_DESC& desc, std::string& outStatus)
{
	try
	{
		if (m_bDirty || Is_Publishing())
		{
			outStatus = "The previous placement draft is preserved. Save it while its Area is active, or explicitly discard the detached draft before binding again.";
			return false;
		}
		IMapAuthoringHost* host = Find_ActiveMapAuthoringHost();
		if (nullptr == host)
		{
			outStatus = "The current Level owns no live map. Enter the Level that owns this Area.";
			return false;
		}
		if (host->Get_MapAuthoringCatalog().Get_AreaId() != desc.areaId)
		{
			outStatus = "The current Level owns " + host->Get_MapAuthoringCatalog().Get_AreaId() +
				"; enter the Level that owns " + desc.areaId + " to edit its placements.";
			return false;
		}
		if (desc.sourceCatalog.empty() || desc.sourcePlacements.empty())
		{
			outStatus = "MapCatalog declares no authoring catalog/placement pair for " + desc.areaId + ".";
			return false;
		}

		CMapAssetCatalog catalog;
		if (!catalog.Load_SourceMetadata(desc.sourceCatalog, desc.sourcePlacements,
			desc.areaId, desc.sourceMaterials))
		{
			outStatus = catalog.Get_Status();
			return false;
		}
		std::string readOnlyReason;
		/* Read placement sidecars from the loaded runtime owner before parsing
		   the full source rows. No second material DOM or material vector copy. */
		if (!catalog.Bind_RuntimeView(host->Get_MapAuthoringCatalog()))
			readOnlyReason = catalog.Get_Status() + "; the rows are listed for inspection only.";
		std::string bytes;
		if (!ReadDocumentBytes(desc.sourcePlacements, bytes))
		{
			outStatus = "Could not read " + desc.sourcePlacements.filename().string() + " for the save baseline.";
			return false;
		}
		std::vector<MAP_PLACEMENT_RECORD> records;
		if (!CMapPlacementDocument::Read(desc.sourcePlacements, catalog, records, outStatus)) return false;
		std::string parsedBytes;
		if (!ReadDocumentBytes(desc.sourcePlacements, parsedBytes) || parsedBytes != bytes)
		{
			outStatus = "Source changed during binding. The previous draft is preserved.";
			return false;
		}

		std::unordered_map<uint64_t, size_t> index;
		index.reserve(records.size());
		for (size_t row = 0; row < records.size(); ++row) index.emplace(records[row].placementId, row);
		if (readOnlyReason.empty())
		{
			const auto& live = host->Get_MapAuthoringPlacements();
			std::unordered_set<uint64_t> liveIds;
			liveIds.reserve(live.size());
			bool parity = index.size() == records.size() &&
				(desc.allowPartialLive || live.size() == records.size());
			for (const auto& entry : live)
			{
				const auto row = index.find(entry.record.placementId);
				if (row == index.end() || !liveIds.insert(entry.record.placementId).second ||
					records[row->second].assetId != entry.record.assetId)
				{
					parity = false;
					break;
				}
			}
			if (!parity)
				readOnlyReason = "Source/live placement identities differ; inspection remains available. No rows will be replaced.";
		}

		// Allocate indexes, names and diagnostics before detaching a clean draft.
		std::string areaId = desc.areaId;
		std::filesystem::path sourcePlacements = desc.sourcePlacements;
		std::string status = readOnlyReason.empty() ?
			"Editing " + areaId + " on " + std::string(host->Get_MapAuthoringLabel()) + ": " +
			std::to_string(records.size()) + " placements. Select an object, then edit its transform." : readOnlyReason;
		outStatus = status;
		static_assert(std::is_nothrow_move_assignable_v<CMapAssetCatalog>);
		End();
		m_SessionCreated.clear();
        Reset_History();
		m_iNextPlacementId = 1u;
		m_bDirty = false;
		m_bPreserveUnloadedRows = desc.allowPartialLive;
		m_AreaId.swap(areaId);
		m_SourcePlacements.swap(sourcePlacements);
		m_bDeclaresLights = desc.declaresLights;
		m_iLevelIndex = host->Get_MapAuthoringLevelIndex();
		m_Catalog = std::move(catalog);
		m_Draft.swap(records);
		m_DraftIndex.swap(index);
		m_BaselineBytes.swap(bytes);
		m_bReadOnly = !readOnlyReason.empty();
		m_ReadOnlyReason.swap(readOnlyReason);
		m_bRowsDirty = true;
		m_Status.swap(status);
		m_bBound = true;
		return true;
	}
	catch (const std::bad_alloc&)
	{
		// A diagnostic itself must not turn memory pressure into a second throw.
		try { outStatus = "Placement editing ran out of memory. The previous draft is preserved; inspection remains available."; }
		catch (...) {}
		return false;
	}
	catch (const std::exception&)
	{
		try { outStatus = "Placement editing could not load its source. The previous draft is preserved; inspection remains available."; }
		catch (...) {}
		return false;
	}
}

void Client::CMapPlacementEditSession::End()
{
	m_bPickArmed = false;
	Remove_Outline();
	m_bBound = false;
	m_iSelectedPlacementId = 0u;
	m_ModelCache.clear();
}

bool_t Client::CMapPlacementEditSession::Discard_DetachedDraft()
{
    if (m_bBound || Is_Publishing()) return false;
    m_Draft.clear(); m_DraftIndex.clear(); m_SessionCreated.clear();
    Reset_History();
    m_bDirty = false; m_bRowsDirty = true;
    m_Status = "Detached placement draft explicitly discarded.";
    return true;
}

void Client::CMapPlacementEditSession::Update(const bool_t outlineVisible)
{
	Poll_Publish();
	if (!m_bBound)
		return;
	IMapAuthoringHost* host = Find_ActiveMapAuthoringHost();
	if (nullptr == host ||
		host->Get_MapAuthoringLevelIndex() != m_iLevelIndex ||
		host->Get_MapAuthoringCatalog().Get_AreaId() != m_AreaId ||
		(!m_bReadOnly && !m_Catalog.Is_RuntimeViewOf(host->Get_MapAuthoringCatalog())))
	{
		End();
		m_Status = "Editing ended: the current Level no longer owns " + m_AreaId +
			". The unsaved draft is preserved. It must be explicitly discarded before a new binding.";
		return;
	}
	if (!outlineVisible)
	{
		/* The window that owns the selection is closed, so the green box goes
		   with it; reopening rebuilds it from the same selection. */
		Remove_Outline();
		return;
	}
	Refresh_Outline();
}

bool_t Client::CMapPlacementEditSession::Consume_RowsDirty()
{
	const bool_t dirty = m_bRowsDirty;
	m_bRowsDirty = false;
	return dirty;
}

Client::IMapAuthoringHost* Client::CMapPlacementEditSession::Resolve_Host() const
{
	IMapAuthoringHost* host = Find_ActiveMapAuthoringHost();
	if (nullptr == host || !m_bBound ||
		host->Get_MapAuthoringLevelIndex() != m_iLevelIndex ||
		host->Get_MapAuthoringCatalog().Get_AreaId() != m_AreaId ||
		(!m_bReadOnly && !m_Catalog.Is_RuntimeViewOf(host->Get_MapAuthoringCatalog())))
	{
		return nullptr;
	}
	return host;
}

Client::MAP_RUNTIME_PLACED_ENTRY* Client::CMapPlacementEditSession::Find_Entry(
	const uint64_t placementId)
{
	IMapAuthoringHost* host = Resolve_Host();
	if (nullptr == host || 0u == placementId)
		return nullptr;
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& live = host->Get_MapAuthoringPlacements();
	const auto found = std::find_if(live.begin(), live.end(),
		[placementId](const MAP_RUNTIME_PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	return found == live.end() ? nullptr : &*found;
}

const Client::MAP_RUNTIME_PLACED_ENTRY* Client::CMapPlacementEditSession::Find_Entry(
	const uint64_t placementId) const
{
	return const_cast<CMapPlacementEditSession*>(this)->Find_Entry(placementId);
}

const Client::MAP_PLACEMENT_RECORD* Client::CMapPlacementEditSession::Find_Draft(
	const uint64_t placementId) const
{
	const auto found = m_DraftIndex.find(placementId);
	return found == m_DraftIndex.end() ? nullptr : &m_Draft[found->second];
}

void Client::CMapPlacementEditSession::Remember_Draft(
	const MAP_PLACEMENT_RECORD& record)
{
	const auto found = m_DraftIndex.find(record.placementId);
	if (found == m_DraftIndex.end())
	{
		m_DraftIndex.emplace(record.placementId, m_Draft.size());
		m_Draft.push_back(record);
	}
	else
	{
		m_Draft[found->second] = record;
	}
}

void Client::CMapPlacementEditSession::Forget_Draft(const uint64_t placementId)
{
	std::erase_if(m_Draft, [placementId](const MAP_PLACEMENT_RECORD& record)
		{
			return record.placementId == placementId;
		});
	Reindex_Draft();
}

void Client::CMapPlacementEditSession::Reindex_Draft()
{
	m_DraftIndex.clear();
	m_DraftIndex.reserve(m_Draft.size());
	for (size_t index = 0; index < m_Draft.size(); ++index)
		m_DraftIndex.emplace(m_Draft[index].placementId, index);
}

void Client::CMapPlacementEditSession::Rebase_SelfMotions()
{
	if (IMapAuthoringHost* host = Resolve_Host())
		host->Rebase_MapAuthoringSelfMotions(m_Draft);
}

uint64_t Client::CMapPlacementEditSession::Allocate_EditorPlacementId()
{
	IMapAuthoringHost* host = Resolve_Host();
	if (nullptr == host)
		return 0u;
	/* Live entries and the authoring draft agree after a bind, but a draft row
	   can outlive its entry mid edit, so both id sets are checked. */
	const size_t attempts =
		host->Get_MapAuthoringPlacements().size() + m_Draft.size();
	for (size_t attempt = 0; attempt <= attempts; ++attempt)
	{
		if (0u == m_iNextPlacementId ||
			m_iNextPlacementId > CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
			m_iNextPlacementId = 1u;

		const uint64_t candidate = m_iNextPlacementId++;
		if (nullptr == Find_Entry(candidate) && !m_DraftIndex.contains(candidate))
			return candidate;
	}
	return 0u;
}

bool_t Client::CMapPlacementEditSession::Can_ChangeStructure()
{
	IMapAuthoringHost* host = Resolve_Host();
	if (nullptr == host)
	{
		m_Status = "The current Level no longer owns " + m_AreaId + ".";
		return false;
	}
	std::string reason;
	if (!host->Can_ChangeMapAuthoringStructure(reason))
	{
		m_Status = reason.empty() ?
			"Stop active arena/Object/Composition playback before adding or deleting map objects." :
			reason;
		return false;
	}
	return true;
}

void Client::CMapPlacementEditSession::Select_Placement(const uint64_t placementId)
{
	if (m_iSelectedPlacementId != placementId) End_EditGesture();
	m_iSelectedPlacementId = placementId;
}

void Client::CMapPlacementEditSession::Clear_Selection()
{
	End_EditGesture();
	m_iSelectedPlacementId = 0u;
	Remove_Outline();
}

bool_t Client::CMapPlacementEditSession::Is_SessionCreated(
	const uint64_t placementId) const
{
	return std::find(m_SessionCreated.begin(), m_SessionCreated.end(),
		placementId) != m_SessionCreated.end();
}

std::string Client::CMapPlacementEditSession::Describe_EditBlock(
	const uint64_t placementId) const
{
	if (m_bReadOnly)
		return m_ReadOnlyReason;
	const MAP_PLACEMENT_RECORD* draft = Find_Draft(placementId);
	const MAP_RUNTIME_PLACED_ENTRY* entry = Find_Entry(placementId);
	if (nullptr == draft || nullptr == entry)
		return "This placement is not part of the live Level; it stays listed for inspection.";
	if (draft->sourceLevel.starts_with("VALTAN_PHASE_"))
		return "Phase-driven placement (VALTAN_PHASE_): the encounter owns its visibility, so it is not edited here.";
	const MAP_ASSET_ENTRY* asset = m_Catalog.Find(draft->assetId);
	if (nullptr == asset)
		return "Asset is not in the Area catalog: " + draft->assetId;
	if (MAP_ASSET_RENDER_MODE::BACKGROUND == asset->renderProfile.renderMode)
		return "Backdrop asset (BACKGROUND): it encloses the stage, is not pickable in the viewport and is not edited here.";
	bool_t runtimeVisible = draft->visible;
	if (draft->visible &&
		CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, runtimeVisible) &&
		!runtimeVisible)
	{
		return "Hidden by a runtime override (sequence / customizing / floor swap); edits wait until it shows again.";
	}
	return std::string{};
}

void Client::CMapPlacementEditSession::Arm_Pick()
{
	if (!m_bBound)
		return;
	m_bPickArmed = true;
	m_Status = "Pick armed: click a map object in the viewport to select it (Esc / right-click cancels).";
}

void Client::CMapPlacementEditSession::Cancel_Pick(std::string reason)
{
	m_bPickArmed = false;
	if (!reason.empty())
		m_Status = std::move(reason);
}

void Client::CMapPlacementEditSession::Complete_Pick(const float3_t& worldPoint)
{
	m_bPickArmed = false;
	IMapAuthoringHost* host = Resolve_Host();
	if (nullptr == host)
	{
		m_Status = "The current Level no longer owns " + m_AreaId + "; the pick was dropped.";
		return;
	}
	uint64_t placementId = 0u;
	size_t containingCount = 0u;
	if (!CMapPlacementRuntime::Try_Resolve_PickedPlacement(
		m_iLevelIndex, m_Catalog, host->Get_MapAuthoringPlacements(),
		m_ModelCache, worldPoint, placementId, containingCount))
	{
		m_Status = "Picked geometry is not a map placement of " + m_AreaId +
			"; selection unchanged (Pick World Object again).";
		return;
	}
	Select_Placement(placementId);
	const MAP_PLACEMENT_RECORD* draft = Find_Draft(placementId);
	const MAP_ASSET_ENTRY* asset = nullptr == draft ?
		nullptr : m_Catalog.Find(draft->assetId);
	std::string status = "Picked placement #" + std::to_string(placementId) + " (" +
		(nullptr != asset ? asset->label :
			(nullptr != draft ? draft->assetId : std::string("?"))) + ")";
	if (containingCount > 1u)
	{
		status += "; " + std::to_string(containingCount) +
			" nested bounds contain the point, the smallest was chosen - use the list to pick another";
	}
	else if (0u == containingCount)
	{
		status += " by its nearest bounding sphere";
	}
	m_Status = status + ".";
}

bool_t Client::CMapPlacementEditSession::Apply_Transform(
	const uint64_t placementId, const MAP_PLACEMENT_RECORD& staged)
{
	const std::string blocked = Describe_EditBlock(placementId);
	if (!blocked.empty())
	{
		m_Status = blocked;
		return false;
	}
	IMapAuthoringHost* host = Resolve_Host();
	MAP_RUNTIME_PLACED_ENTRY* entry = Find_Entry(placementId);
	if (nullptr == host || nullptr == entry)
	{
		m_Status = "The current Level no longer owns " + m_AreaId + ".";
		return false;
	}
    const auto* draft = Find_Draft(placementId);
    if (!draft) return false;
    if (SamePose(*draft, staged) && draft->visible == staged.visible) return true;
    MAP_PLACEMENT_RECORD before, after, draftAfter;
    try
    {
        before = *draft; after = draftAfter = staged;
        if (!m_HistoryReplay) m_UndoHistory.reserve(m_UndoHistory.size() + 1u);
    }
    catch (const std::bad_alloc&)
    { m_Status = "Transform history allocation failed; the draft and runtime were preserved."; return false; }
	const bool_t wasBatched = nullptr != entry->batch;
	const bool_t parityFlip = wasBatched &&
		IsMirrored(entry->record.signedScale) != IsMirrored(staged.signedScale);
	std::string applyStatus;
	if (CMapPlacementRuntime::PLACEMENT_TRANSFORM_RESULT::APPLIED !=
		CMapPlacementRuntime::Apply_PlacementTransform(
			m_iLevelIndex, m_Catalog, m_ModelCache, *entry, staged, applyStatus))
	{
		m_Status = applyStatus + (parityFlip ?
			" (mirror parity migration failed; the batched instance stays visible)." :
			(wasBatched ? " (the batched instance stays visible)." : "."));
		return false;
	}
    m_Draft[m_DraftIndex.at(placementId)] = std::move(draftAfter);
    Record_Edit(std::move(before), std::move(after), m_iSelectedPlacementId, m_iSelectedPlacementId);
	m_bDirty = true;
	Rebase_SelfMotions();
	return true;
}

bool_t Client::CMapPlacementEditSession::Duplicate_Selected()
{
	if (m_bReadOnly)
	{
		m_Status = m_ReadOnlyReason;
		return false;
	}
	if (!Can_ChangeStructure())
		return false;
	IMapAuthoringHost* host = Resolve_Host();
	const MAP_PLACEMENT_RECORD* source = Find_Draft(m_iSelectedPlacementId);
	if (nullptr == host || nullptr == source ||
		nullptr == Find_Entry(m_iSelectedPlacementId))
	{
		m_Status = "Select a placed object before duplicating.";
		return false;
	}
	const MAP_ASSET_ENTRY* asset = m_Catalog.Find(source->assetId);
	if (nullptr == asset)
	{
		m_Status = "Duplicate rejected: asset is not in the catalog: " + source->assetId;
		return false;
	}
	const uint64_t placementId = Allocate_EditorPlacementId();
	if (0u == placementId)
	{
		m_Status = "No editor placement ID is available";
		return false;
	}

	/* Asset, pose, visibility and baked lighting come from the authored
	   record; only the identity is new, so the publisher accepts the row
	   exactly like a freshly placed editor object. */
	MAP_PLACEMENT_RECORD record = *source;
	const uint64_t sourceId = record.placementId;
	record.placementId = placementId;
	record.sourcePlacementId = "editor:" + m_AreaId + ":" + std::to_string(placementId);
	record.sourceLevel = "EDITOR";
	record.transformSource = "editor";

    std::vector<MAP_PLACEMENT_RECORD> stagedDraft;
    std::unordered_map<uint64_t, size_t> stagedIndex;
    std::vector<uint64_t> stagedCreated;
    MAP_PLACEMENT_RECORD historyAfter;
    try
    {
        stagedDraft = m_Draft; stagedIndex = m_DraftIndex; stagedCreated = m_SessionCreated;
        stagedIndex.emplace(placementId, stagedDraft.size()); stagedDraft.push_back(record);
        stagedCreated.push_back(placementId); historyAfter = record;
        host->Get_MapAuthoringPlacements().reserve(host->Get_MapAuthoringPlacements().size() + 1u);
        m_UndoHistory.reserve(m_UndoHistory.size() + 1u);
    }
    catch (const std::bad_alloc&)
    { m_Status = "Duplicate staging failed; the draft and runtime were preserved."; return false; }
	MAP_RUNTIME_PLACED_ENTRY placed{};
	if (!CMapPlacementRuntime::Create_Placement(
		m_iLevelIndex, m_Catalog, record, placed))
	{
		m_Status = "Failed to clone map object for " + asset->id;
		return false;
	}
	host->Get_MapAuthoringPlacements().push_back(std::move(placed));
    m_Draft.swap(stagedDraft); m_DraftIndex.swap(stagedIndex); m_SessionCreated.swap(stagedCreated);
	m_iSelectedPlacementId = placementId;
    End_EditGesture();
    Record_Edit(std::nullopt, std::move(historyAfter), sourceId, placementId);
	m_bDirty = true;
	m_bRowsDirty = true;
	Rebase_SelfMotions();
	m_Status = "Duplicated placement #" + std::to_string(sourceId) + " as #" +
		std::to_string(placementId) + " (" + asset->label +
		") at the same pose; drag Position to move it.";
	return true;
}

bool_t Client::CMapPlacementEditSession::Delete_Selected()
{
	if (m_bReadOnly)
	{
		m_Status = m_ReadOnlyReason;
		return false;
	}
	const uint64_t placementId = m_iSelectedPlacementId;
	if (!Is_SessionCreated(placementId))
	{
		m_Status = "Delete removes only session duplicates. Use Visible to retain the identity of an original placement.";
		return false;
	}
	if (!Can_ChangeStructure())
		return false;
	IMapAuthoringHost* host = Resolve_Host();
	if (nullptr == host)
	{
		m_Status = "The current Level no longer owns " + m_AreaId + ".";
		return false;
	}
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& live = host->Get_MapAuthoringPlacements();
	const auto found = std::find_if(live.begin(), live.end(),
		[placementId](const MAP_RUNTIME_PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	if (found == live.end())
	{
		m_Status = "The duplicate is no longer in the live Level.";
		return false;
	}
    const auto* authored = Find_Draft(placementId);
    if (!authored) { m_Status = "The duplicate has no authoring record; deletion was not applied."; return false; }
    MAP_PLACEMENT_RECORD deleted;
    std::vector<MAP_PLACEMENT_RECORD> stagedDraft;
    std::unordered_map<uint64_t, size_t> stagedIndex;
    std::vector<uint64_t> stagedCreated;
    try
    {
        deleted = *authored; stagedDraft = m_Draft; stagedCreated = m_SessionCreated;
        std::erase_if(stagedDraft, [placementId](const auto& row) { return row.placementId == placementId; });
        stagedIndex.reserve(stagedDraft.size());
        for (size_t i = 0u; i < stagedDraft.size(); ++i) stagedIndex.emplace(stagedDraft[i].placementId, i);
        std::erase(stagedCreated, placementId);
        if (!m_HistoryReplay) m_UndoHistory.reserve(m_UndoHistory.size() + 1u);
    }
    catch (const std::bad_alloc&)
    { m_Status = "Delete staging failed; the draft and runtime were preserved."; return false; }
	if (nullptr != found->object)
	{
		if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iLevelIndex, found->layerTag,
			static_pointer_cast<CGameObject>(found->object))))
		{
			m_Status = "Delete failed: the duplicate stays in the Level.";
			return false;
		}
	}
	else if (nullptr != found->batch)
	{
		if (FAILED(found->batch->Set_InstanceVisible(placementId, false)))
		{
			m_Status = "Delete failed: the batched instance stays visible.";
			return false;
		}
	}
	else
	{
		m_Status = "Delete failed: the duplicate has no runtime representation.";
		return false;
	}
	live.erase(found);
    m_Draft.swap(stagedDraft); m_DraftIndex.swap(stagedIndex); m_SessionCreated.swap(stagedCreated);
	Clear_Selection();
    Record_Edit(std::move(deleted), std::nullopt, placementId, 0u);
	m_bDirty = true;
	m_bRowsDirty = true;
	Rebase_SelfMotions();
	m_Status = "Deleted the duplicate #" + std::to_string(placementId) + ".";
	return true;
}

void Client::CMapPlacementEditSession::Reset_History()
{
    m_UndoHistory.clear(); m_RedoHistory.clear();
    m_HistoryRevision = m_SavedRevision = m_NextRevision = m_RollbackRevision = 0u;
    m_HistoryReplay = false; End_EditGesture();
}

void Client::CMapPlacementEditSession::Record_Edit(
    std::optional<MAP_PLACEMENT_RECORD> before, std::optional<MAP_PLACEMENT_RECORD> after,
    uint64_t selectedBefore, uint64_t selectedAfter)
{
    if (m_HistoryReplay) return;
    const uint64_t revision = ++m_NextRevision;
    if (m_HistoryGesture && m_GestureRecorded && before && after && !m_UndoHistory.empty() &&
        m_UndoHistory.back().before && m_UndoHistory.back().after &&
        m_UndoHistory.back().after->placementId == before->placementId)
    {
        auto& edit = m_UndoHistory.back();
        edit.after = std::move(after); edit.selectedAfter = selectedAfter; edit.revisionAfter = revision;
    }
    else
    {
        m_UndoHistory.push_back({std::move(before), std::move(after), selectedBefore,
            selectedAfter, m_HistoryRevision, revision});
        if (m_UndoHistory.size() > 64u) m_UndoHistory.erase(m_UndoHistory.begin());
    }
    m_HistoryRevision = revision; m_GestureRecorded = m_HistoryGesture;
    m_RedoHistory.clear();
}

bool Client::CMapPlacementEditSession::Undo() { return Apply_History(false); }
bool Client::CMapPlacementEditSession::Redo() { return Apply_History(true); }

bool Client::CMapPlacementEditSession::Apply_History(bool redo)
{
    if (redo ? !Can_Redo() : !Can_Undo())
    { m_Status = "No placement history is available, or publishing is in progress."; return false; }
    if (!Can_ChangeStructure()) return false;
    auto& source = redo ? m_RedoHistory : m_UndoHistory;
    auto& destination = redo ? m_UndoHistory : m_RedoHistory;
    EDIT edit;
    try { edit = source.back(); destination.reserve(destination.size() + 1u); }
    catch (const std::bad_alloc&)
    { m_Status = "Placement history allocation failed; the draft and history were preserved."; return false; }
    const auto& expected = redo ? edit.before : edit.after;
    const auto& target = redo ? edit.after : edit.before;
    const uint64_t id = expected ? expected->placementId : target->placementId;
    auto* host = Resolve_Host();
    const auto* current = Find_Draft(id);
    if (!host || (expected ? (!current || current->assetId != expected->assetId ||
        current->sourcePlacementId != expected->sourcePlacementId ||
        !SamePose(*current, *expected) || current->visible != expected->visible) : current != nullptr))
    { m_Status = "Placement history conflicts with the current draft; selection and history were preserved."; return false; }
    const uint64_t previousSelection = m_iSelectedPlacementId;
    bool applied = false;
    End_EditGesture();
    m_HistoryReplay = true;
    if (expected && target) applied = Apply_Transform(id, *target);
    else if (expected)
    {
        m_iSelectedPlacementId = id;
        applied = Delete_Selected();
    }
    else if (target && !Find_Entry(id) && CMapPlacementDocument::Is_Valid(*target, m_Catalog))
    {
        std::vector<MAP_PLACEMENT_RECORD> stagedDraft;
        std::unordered_map<uint64_t, size_t> stagedIndex;
        std::vector<uint64_t> stagedCreated;
        try
        {
            stagedDraft = m_Draft; stagedIndex = m_DraftIndex; stagedCreated = m_SessionCreated;
            stagedIndex.emplace(id, stagedDraft.size()); stagedDraft.push_back(*target); stagedCreated.push_back(id);
            host->Get_MapAuthoringPlacements().reserve(host->Get_MapAuthoringPlacements().size() + 1u);
        }
        catch (const std::bad_alloc&)
        {
            m_HistoryReplay = false;
            m_Status = "Placement recreation staging failed; the draft and history were preserved.";
            return false;
        }
        MAP_RUNTIME_PLACED_ENTRY placed{};
        if (CMapPlacementRuntime::Create_Placement(m_iLevelIndex, m_Catalog, *target, placed))
        {
            host->Get_MapAuthoringPlacements().push_back(std::move(placed));
            m_Draft.swap(stagedDraft); m_DraftIndex.swap(stagedIndex); m_SessionCreated.swap(stagedCreated);
            Rebase_SelfMotions(); applied = true;
        }
        else m_Status = "Could not recreate the deleted placement; the draft and history were preserved.";
    }
    else m_Status = "Placement history target is invalid or its stable ID is already live.";
    m_HistoryReplay = false;
    if (!applied) { m_iSelectedPlacementId = previousSelection; return false; }
    m_HistoryRevision = redo ? edit.revisionAfter : edit.revisionBefore;
    m_iSelectedPlacementId = redo ? edit.selectedAfter : edit.selectedBefore;
    destination.push_back(std::move(edit)); source.pop_back();
    m_bDirty = m_HistoryRevision != m_SavedRevision; m_bRowsDirty = true;
    m_Status = redo ? "Placement edit redone; selection restored." : "Placement edit undone; selection restored.";
    return true;
}

std::filesystem::path Client::CMapPlacementEditSession::Rollback_CopyPath() const
{
	std::filesystem::path rollback = m_SourcePlacements;
	rollback += L"." + std::to_wstring(GetCurrentProcessId()) + L".worldlevel.rollback";
	return rollback;
}

bool_t Client::CMapPlacementEditSession::Save()
{
	if (m_bReadOnly)
	{
		m_Status = m_ReadOnlyReason;
		return false;
	}
	if (m_PublishRunner.Is_Running())
	{
		m_Status = "Publish in progress; Save again when it finishes.";
		return false;
	}
	IMapAuthoringHost* host = Resolve_Host();
	if (nullptr == host)
	{
		m_Status = "The current Level no longer owns " + m_AreaId + "; nothing was written.";
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> document;
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& live = host->Get_MapAuthoringPlacements();
	if (m_bPreserveUnloadedRows) document = m_Draft;
	else document.reserve(live.size());
	std::unordered_set<uint64_t> liveIds;
	for (const MAP_RUNTIME_PLACED_ENTRY& entry : live)
	{
		const bool_t hasObject = nullptr != entry.object;
		const bool_t hasBatch = nullptr != entry.batch;
		if (hasObject == hasBatch ||
			!liveIds.insert(entry.record.placementId).second ||
			nullptr == m_Catalog.Find(entry.record.assetId))
		{
			m_Status = "Save aborted: runtime representation is invalid for placement #" +
				std::to_string(entry.record.placementId) + ".";
			return false;
		}
		/* The authored record, never the live one: a self motion or a sequence
		   samples the live entry every frame. */
		const MAP_PLACEMENT_RECORD* draft = Find_Draft(entry.record.placementId);
        if (m_bPreserveUnloadedRows && (nullptr == draft || draft->assetId != entry.record.assetId))
        { m_Status = "Save aborted: live/source identity changed. Draft preserved."; return false; }
		MAP_PLACEMENT_RECORD stored = nullptr != draft ? *draft : entry.record;
		if (stored.sourceLevel.starts_with("VALTAN_PHASE_"))
			stored.visible = false;
		if (!CMapPlacementDocument::Is_Valid(stored, m_Catalog))
		{
			m_Status = "Save aborted: placement #" +
				std::to_string(stored.placementId) + " has an invalid transform.";
			return false;
		}
        if (!m_bPreserveUnloadedRows) document.push_back(std::move(stored));
    }
    for (auto& stored : document)
    {
        if (stored.sourceLevel.starts_with("VALTAN_PHASE_")) stored.visible = false;
        if (!CMapPlacementDocument::Is_Valid(stored, m_Catalog))
        { m_Status = "Save aborted: an unloaded source row failed validation. Draft preserved."; return false; }
    }

	std::string currentBytes;
	if (!ReadDocumentBytes(m_SourcePlacements, currentBytes))
	{
		m_Status = "Save conflict: could not read the placement document on disk. Draft preserved.";
		return false;
	}
	if (currentBytes != m_BaselineBytes)
	{
		m_Status = "Save conflict: map source changed on disk. Draft preserved; Reload before saving.";
		return false;
	}
	const std::filesystem::path rollback = Rollback_CopyPath();
	if (!WriteDocumentBytes(rollback, currentBytes))
	{
		m_Status = "Save aborted: could not write the rollback copy " + rollback.string();
		return false;
	}

    std::string latestBytes;
    if (!ReadDocumentBytes(m_SourcePlacements, latestBytes) || latestBytes != currentBytes)
    { m_Status = "Save conflict: concurrent source change after backup. Draft preserved."; return false; }
	std::string writeStatus;
	if (!CMapPlacementDocument::Write(m_SourcePlacements, m_AreaId,
		document, m_Catalog, writeStatus, nullptr, &currentBytes))
	{
		std::error_code removeError;
		std::filesystem::remove(rollback, removeError);
		m_Status = writeStatus;
		return false;
	}
	std::string savedBytes;
	if (!ReadDocumentBytes(m_SourcePlacements, savedBytes))
	{
		std::error_code removeError;
		std::filesystem::remove(rollback, removeError);
		m_Status = "Saved " + m_SourcePlacements.string() +
			", but its bytes could not be read back; publish was not started.";
		return false;
	}
	m_BaselineBytes = savedBytes;
    End_EditGesture();
    m_RollbackRevision = m_SavedRevision;
    m_SavedRevision = m_HistoryRevision;
	m_bDirty = false;
	m_RollbackBytes = std::move(currentBytes);
	m_bRollbackValid = true;

	std::string publishStatus;
	if (!m_PublishRunner.Start(m_AreaId, "Placements", L"WorldSceneTool-Placements", publishStatus))
	{
		/* The authoring file is saved; only the runtime rebuild did not start,
		   so the pre-save copy is not needed any more. */
		std::error_code removeError;
		std::filesystem::remove(rollback, removeError);
		m_bRollbackValid = false;
		m_RollbackBytes.clear();
		m_Status = "Saved " + m_SourcePlacements.string() +
			". Publish did not start: " + publishStatus +
			" Run Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId " + m_AreaId +
			" -Scope Placements -Mode Publish, then reload the Client.";
		return true;
	}
	m_Status = "Saved " + m_SourcePlacements.string() + " (" +
		std::to_string(document.size()) + " placements); publishing " + m_AreaId + "...";
	return true;
}

void Client::CMapPlacementEditSession::Poll_Publish()
{
	bool_t succeeded = false;
	uint32_t exitCode = 0u;
	if (!m_PublishRunner.Poll(succeeded, exitCode))
		return;
	const std::string areaId = m_PublishRunner.Get_AreaId();
	const std::string logPath = m_PublishRunner.Get_LogPath().string();
	const std::filesystem::path rollback = m_bRollbackValid ?
		Rollback_CopyPath() : std::filesystem::path{};
	if (succeeded)
	{
		if (!rollback.empty())
		{
			std::error_code removeError;
			std::filesystem::remove(rollback, removeError);
		}
		m_bRollbackValid = false;
		m_RollbackBytes.clear();
		std::string produced = "Client/Bin/DataFiles/Map/" + areaId + ".mapplacements";
		m_Status = "Published " + areaId + ": " + produced +
			". Commit both LFS files together: Data/Maps/Authoring/" + areaId + "/" +
			areaId + ".mapplacements and Client/Bin/DataFiles/Map/" + areaId +
			".mapplacements. The next Client run loads the runtime file. Log: " + logPath;
		return;
	}

	/* Put the source back so authoring and runtime cannot disagree; the
	   publisher already rolled its own runtime files back. */
	std::string restore;
	if (m_bRollbackValid)
	{
		std::string current;
		if (ReadDocumentBytes(m_SourcePlacements, current) && current == m_BaselineBytes)
		{
			if (WriteDocumentBytesAtomic(m_SourcePlacements, m_RollbackBytes))
			{
				m_BaselineBytes = m_RollbackBytes;
                m_SavedRevision = m_RollbackRevision;
				m_bDirty = m_HistoryRevision != m_SavedRevision;
				std::error_code removeError;
				std::filesystem::remove(rollback, removeError);
				restore = "Authoring file restored to its pre-save bytes; the draft stays dirty for another Save.";
			}
			else
			{
				restore = "Could not restore the authoring file; its pre-save copy remains at " +
					rollback.string() + ".";
			}
		}
		else
		{
			restore = "Authoring file changed since the Save, so it was left as is; the pre-save copy remains at " +
				rollback.string() + ".";
		}
	}
	m_bRollbackValid = false;
	m_RollbackBytes.clear();
	m_Status = "Publish failed for " + areaId + " (exit " + std::to_string(exitCode) +
		"); the publisher rolled its runtime files back. " + restore +
		" Retry with Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId " + areaId +
		" -Scope Placements -Mode Publish. Log: " + logPath;
}

void Client::CMapPlacementEditSession::Refresh_Outline()
{
	MAP_RUNTIME_PLACED_ENTRY* entry = Find_Entry(m_iSelectedPlacementId);
	if (nullptr == entry || m_iLevelIndex >= ETOUI(LEVEL::END))
	{
		if (nullptr != m_Outline.object || 0u != m_Outline.placementId)
			Remove_Outline();
		return;
	}
	/* Self motion, sequences and the transform widgets all write entry.record,
	   so a changed pose is enough to know the box has to follow. */
	if (entry->record.placementId == m_Outline.placementId &&
		SamePose(entry->record, m_Outline.pose))
	{
		return;
	}

	const MAP_ASSET_ENTRY* asset = m_Catalog.Find(entry->record.assetId);
	const shared_ptr<Engine::CModel> model = nullptr == asset ? nullptr :
		CMapPlacementRuntime::Find_PlacementModel(
			m_iLevelIndex, m_Catalog, entry->record.assetId, m_ModelCache);
	float3_t minimum{};
	float3_t maximum{};
	if (nullptr == asset || nullptr == model ||
		!CMapPlacementRuntime::Try_Get_PlacementWorldBounds(
			*asset, model, entry->record, minimum, maximum))
	{
		/* Remember the attempt so it is not retried every frame. */
		Remove_Outline();
		m_Outline.placementId = entry->record.placementId;
		m_Outline.pose = entry->record;
		return;
	}

	CTrigger_Box::TRIGGER_BOX_DESC desc{};
	desc.placementId = std::to_string(entry->record.placementId);
	desc.position = float3_t(
		(minimum.x + maximum.x) * 0.5f,
		(minimum.y + maximum.y) * 0.5f,
		(minimum.z + maximum.z) * 0.5f);
	desc.halfExtents = float3_t(
		(std::max)((maximum.x - minimum.x) * 0.5f, 0.01f),
		(std::max)((maximum.y - minimum.y) * 0.5f, 0.01f),
		(std::max)((maximum.z - minimum.z) * 0.5f, 0.01f));
	desc.yawDegrees = 0.f;
	/* Enabled, non-collision: the green of the wire box palette. */
	desc.isEnabled = true;
	desc.isCollisionBox = false;

	if (nullptr != m_Outline.object &&
		m_Outline.placementId == entry->record.placementId)
	{
		(void)m_Outline.object->Apply_Descriptor(desc);
		m_Outline.pose = entry->record;
		m_Outline.object->Set_AuthoringVisible(true);
		return;
	}

	Remove_Outline();
	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		m_iLevelIndex,
		TEXT("Prototype_GameObject_TriggerBox"),
		m_iLevelIndex,
		TEXT("Layer_TriggerBoxes"),
		&desc,
		&gameObject)))
	{
		m_Outline.placementId = entry->record.placementId;
		m_Outline.pose = entry->record;
		m_Status = "Selected #" + desc.placementId +
			"; outline unavailable (TriggerBox prototype is not registered for this Level).";
		return;
	}
	shared_ptr<CTrigger_Box> box = dynamic_pointer_cast<CTrigger_Box>(gameObject);
	if (nullptr == box)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iLevelIndex, TEXT("Layer_TriggerBoxes"), gameObject);
		m_Outline.placementId = entry->record.placementId;
		m_Outline.pose = entry->record;
		m_Status = "Selected #" + desc.placementId +
			"; outline unavailable (TriggerBox clone type mismatch).";
		return;
	}
	box->Set_Selected(false);
	box->Set_AuthoringVisible(true);
	m_Outline.placementId = entry->record.placementId;
	m_Outline.pose = entry->record;
	m_Outline.object = std::move(box);
}

void Client::CMapPlacementEditSession::Remove_Outline()
{
	if (nullptr != m_Outline.object && m_iLevelIndex < ETOUI(LEVEL::END) &&
		CGameInstance::Get().Get_CurrentLevelID() == m_iLevelIndex)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iLevelIndex,
			TEXT("Layer_TriggerBoxes"),
			static_pointer_cast<CGameObject>(m_Outline.object));
	}
	m_Outline = OUTLINE{};
}
#endif
