#include "MapPlacementEditSession.h"

#ifdef _DEBUG
#include "GameInstance.h"
#include "MapAssetObject.h"
#include "MapAuthoringHost.h"
#include "MapStaticBatchObject.h"
#include "Trigger_Box.h"

#include <algorithm>
#include <cmath>
#include <fstream>
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
	End();
	/* A Level round trip rebuilt the live placements, so a draft kept from an
	   earlier binding cannot be re-attached to them; say so instead of
	   silently replacing it. */
	const bool_t discardedDraft = m_bDirty && !m_Draft.empty();
	m_Draft.clear();
	m_DraftIndex.clear();
	m_SessionCreated.clear();
	m_Catalog = CMapAssetCatalog{};
	m_bDirty = false;
	m_bReadOnly = false;
	m_ReadOnlyReason.clear();
	m_BaselineBytes.clear();
	m_iSelectedPlacementId = 0u;
	m_iNextPlacementId = 1u;

	IMapAuthoringHost* host = Find_ActiveMapAuthoringHost();
	if (nullptr == host)
	{
		outStatus = "The current Level owns no live map. Enter the Level that owns this Area.";
		return false;
	}
	if (host->Get_MapAuthoringCatalog().Get_AreaId() != desc.areaId)
	{
		outStatus = "The current Level owns " +
			host->Get_MapAuthoringCatalog().Get_AreaId() +
			"; enter the Level that owns " + desc.areaId + " to edit its placements.";
		return false;
	}
	if (desc.sourceCatalog.empty() || desc.sourcePlacements.empty())
	{
		outStatus = "MapCatalog declares no authoring catalog/placement pair for " + desc.areaId + ".";
		return false;
	}

	CMapAssetCatalog catalog;
	if (!catalog.Load_Source(desc.sourceCatalog, desc.sourcePlacements,
		desc.areaId, desc.sourceMaterials))
	{
		outStatus = catalog.Get_Status();
		return false;
	}
	std::vector<MAP_PLACEMENT_RECORD> records;
	if (!CMapPlacementDocument::Read(desc.sourcePlacements, catalog, records, outStatus))
		return false;
	std::string bytes;
	if (!ReadDocumentBytes(desc.sourcePlacements, bytes))
	{
		outStatus = "Could not read " + desc.sourcePlacements.filename().string() +
			" for the save baseline.";
		return false;
	}

	std::string readOnlyReason;
	/* The live prototypes belong to the Level; binding them keeps one catalog
	   for both the document writer and the runtime clone. */
	if (!catalog.Bind_RuntimePrototypes(host->Get_MapAuthoringCatalog()))
		readOnlyReason = catalog.Get_Status() +
			"; the rows are listed for inspection only.";

	if (readOnlyReason.empty())
	{
		/* A Level that loaded a filtered scope cannot be saved from here: the
		   document would drop every row the Level did not load. */
		const std::vector<MAP_RUNTIME_PLACED_ENTRY>& live =
			host->Get_MapAuthoringPlacements();
		std::unordered_set<uint64_t> sourceIds;
		sourceIds.reserve(records.size());
		for (const MAP_PLACEMENT_RECORD& record : records)
			sourceIds.insert(record.placementId);
		bool_t parity = live.size() == records.size() &&
			sourceIds.size() == records.size();
		if (parity)
		{
			for (const MAP_RUNTIME_PLACED_ENTRY& entry : live)
			{
				if (!sourceIds.contains(entry.record.placementId))
				{
					parity = false;
					break;
				}
			}
		}
		if (!parity)
			readOnlyReason = "The live Level holds " + std::to_string(live.size()) +
				" placements but the authoring document has " +
				std::to_string(records.size()) +
				"; read-only inspection until they match (load scope or an external edit).";
	}

	m_bBound = true;
	m_AreaId = desc.areaId;
	m_SourcePlacements = desc.sourcePlacements;
	m_bDeclaresLights = desc.declaresLights;
	m_iLevelIndex = host->Get_MapAuthoringLevelIndex();
	m_Catalog = std::move(catalog);
	m_Draft = std::move(records);
	Reindex_Draft();
	m_BaselineBytes = std::move(bytes);
	m_bReadOnly = !readOnlyReason.empty();
	m_ReadOnlyReason = std::move(readOnlyReason);
	m_bRowsDirty = true;
	m_Status = m_bReadOnly ? m_ReadOnlyReason :
		"Editing " + m_AreaId + " on " + std::string(host->Get_MapAuthoringLabel()) +
		": " + std::to_string(m_Draft.size()) +
		" placements. Pick World Object, or select a row, then drag its transform.";
	if (discardedDraft)
		m_Status += " The unsaved draft of the previous binding was dropped: the Level rebuilt its placements, so the saved document is the only pose the live objects share.";
	return true;
}

void Client::CMapPlacementEditSession::End()
{
	m_bPickArmed = false;
	Remove_Outline();
	m_bBound = false;
	m_iSelectedPlacementId = 0u;
	m_ModelCache.clear();
}

void Client::CMapPlacementEditSession::Update(const bool_t outlineVisible)
{
	Poll_Publish();
	if (!m_bBound)
		return;
	IMapAuthoringHost* host = Find_ActiveMapAuthoringHost();
	if (nullptr == host ||
		host->Get_MapAuthoringLevelIndex() != m_iLevelIndex ||
		host->Get_MapAuthoringCatalog().Get_AreaId() != m_AreaId)
	{
		End();
		m_Status = "Editing ended: the current Level no longer owns " + m_AreaId +
			". The draft is kept in memory; re-enter the Area and press Edit placements again.";
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
		host->Get_MapAuthoringCatalog().Get_AreaId() != m_AreaId)
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
	m_iSelectedPlacementId = placementId;
}

void Client::CMapPlacementEditSession::Clear_Selection()
{
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
	Remember_Draft(entry->record);
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

	MAP_RUNTIME_PLACED_ENTRY placed{};
	if (!CMapPlacementRuntime::Create_Placement(
		m_iLevelIndex, m_Catalog, record, placed))
	{
		m_Status = "Failed to clone map object for " + asset->id;
		return false;
	}
	host->Get_MapAuthoringPlacements().push_back(std::move(placed));
	Remember_Draft(record);
	m_SessionCreated.push_back(placementId);
	m_iSelectedPlacementId = placementId;
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
		m_Status = "Delete removes only the objects duplicated in this editing session; use Map Tool for authored rows.";
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
	Forget_Draft(placementId);
	std::erase(m_SessionCreated, placementId);
	Clear_Selection();
	m_bDirty = true;
	m_bRowsDirty = true;
	Rebase_SelfMotions();
	m_Status = "Deleted the duplicate #" + std::to_string(placementId) + ".";
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
	document.reserve(live.size());
	for (const MAP_RUNTIME_PLACED_ENTRY& entry : live)
	{
		const bool_t hasObject = nullptr != entry.object;
		const bool_t hasBatch = nullptr != entry.batch;
		if (hasObject == hasBatch ||
			nullptr == m_Catalog.Find(entry.record.assetId))
		{
			m_Status = "Save aborted: runtime representation is invalid for placement #" +
				std::to_string(entry.record.placementId) + ".";
			return false;
		}
		/* The authored record, never the live one: a self motion or a sequence
		   samples the live entry every frame. */
		const MAP_PLACEMENT_RECORD* draft = Find_Draft(entry.record.placementId);
		MAP_PLACEMENT_RECORD stored = nullptr != draft ? *draft : entry.record;
		if (stored.sourceLevel.starts_with("VALTAN_PHASE_"))
			stored.visible = false;
		if (!CMapPlacementDocument::Is_Valid(stored, m_Catalog))
		{
			m_Status = "Save aborted: placement #" +
				std::to_string(stored.placementId) + " has an invalid transform.";
			return false;
		}
		document.push_back(std::move(stored));
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

	std::string writeStatus;
	if (!CMapPlacementDocument::Write(m_SourcePlacements, m_AreaId,
		document, m_Catalog, writeStatus, nullptr))
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
	m_bDirty = false;
	m_RollbackBytes = std::move(currentBytes);
	m_bRollbackValid = true;

	std::string publishStatus;
	if (!m_PublishRunner.Start(m_AreaId, "Area", L"WorldLevelTool-Area", publishStatus))
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
			" -Scope Area -Mode Publish, then reload the Client.";
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
		std::string produced = "Client/Bin/DataFiles/Map/" + areaId +
			".mapplacements and " + areaId + ".mapassets";
		if (m_bDeclaresLights)
			produced += " and " + areaId + ".maplights.json";
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
				m_bDirty = true;
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
		" -Scope Area -Mode Publish. Log: " + logPath;
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
