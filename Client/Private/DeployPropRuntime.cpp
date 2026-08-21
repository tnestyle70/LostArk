#include "DeployPropRuntime.h"

#include "DeployPropObject.h"
#include "GameInstance.h"

#include <algorithm>
#include <utility>

namespace
{
	constexpr const wchar_t* DEPLOY_PROP_PROTOTYPE =
		TEXT("Prototype_GameObject_DeployProp");
	constexpr const wchar_t* DEPLOY_PROP_LAYER =
		TEXT("Layer_DeployProps");
}

CDeployPropRuntime::~CDeployPropRuntime()
{
	Clear();
}

CDeployPropRuntime::CDeployPropRuntime(CDeployPropRuntime&& other) noexcept
{
	Move_From(std::move(other));
}

CDeployPropRuntime& CDeployPropRuntime::operator=(
	CDeployPropRuntime&& other) noexcept
{
	if (this == &other)
		return *this;

	Clear();
	Move_From(std::move(other));
	return *this;
}

bool_t CDeployPropRuntime::Load_Area(
	const uint32_t levelIndex,
	const std::string& areaId)
{
	CDeployPropCatalog catalog;
	if (!catalog.Load_Default(areaId))
	{
		m_Status = catalog.Get_Status();
		return false;
	}
	return Load(levelIndex, std::move(catalog));
}

bool_t CDeployPropRuntime::Load(
	const uint32_t levelIndex,
	CDeployPropCatalog catalog)
{
	if (levelIndex >= ETOUI(LEVEL::END) || !catalog.Is_Ready() ||
		catalog.Get_Placements().empty())
	{
		m_Status = "DeployProp runtime input is invalid";
		return false;
	}

	std::vector<DEPLOY_RUNTIME_ENTRY> stagedEntries;
	std::unordered_map<uint64_t, size_t> stagedIndex;
	stagedEntries.reserve(catalog.Get_Placements().size());
	stagedIndex.reserve(catalog.Get_Placements().size());
	for (const DEPLOY_PROP_PLACEMENT& placement : catalog.Get_Placements())
	{
		const DEPLOY_PROP_ASSET_ENTRY* asset = catalog.Find(placement.assetId);
		if (nullptr == asset)
		{
			Remove_Entries(levelIndex, stagedEntries);
			m_Status = "DeployProp staging lost asset: " + placement.assetId;
			return false;
		}

		CDeployPropObject::DEPLOY_PROP_DESC desc{};
		desc.prototypeLevelIndex = levelIndex;
		desc.placement = placement;
		desc.modelKind = asset->kind;
		desc.intactPrototypeTag = asset->intactPrototypeTag;
		desc.fracturedPrototypeTag = asset->fracturedPrototypeTag;
		desc.emissiveIntensity = asset->emissiveIntensity;
		desc.deferredEmissiveOverlay = asset->deferredEmissiveOverlay;

		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			levelIndex,
			DEPLOY_PROP_PROTOTYPE,
			levelIndex,
			DEPLOY_PROP_LAYER,
			&desc,
			&gameObject)))
		{
			Remove_Entries(levelIndex, stagedEntries);
			m_Status = "DeployProp staging rolled back at: " +
				placement.sourcePlacementId;
			return false;
		}

		shared_ptr<CDeployPropObject> object =
			dynamic_pointer_cast<CDeployPropObject>(gameObject);
		if (nullptr == object)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				levelIndex, DEPLOY_PROP_LAYER, gameObject);
			Remove_Entries(levelIndex, stagedEntries);
			m_Status = "DeployProp clone type mismatch";
			return false;
		}

		const size_t entryIndex = stagedEntries.size();
		if (!stagedIndex.emplace(
			placement.runtimePlacementId, entryIndex).second)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				levelIndex, DEPLOY_PROP_LAYER, gameObject);
			Remove_Entries(levelIndex, stagedEntries);
			m_Status = "DeployProp runtime placement ID is duplicated";
			return false;
		}
		stagedEntries.push_back({ placement, std::move(object) });
	}

	Clear();
	m_iLevelIndex = levelIndex;
	m_Catalog = std::move(catalog);
	m_Entries = std::move(stagedEntries);
	m_EntryIndex = std::move(stagedIndex);

	const size_t bindPoseOnly = static_cast<size_t>(std::count_if(
		m_Entries.begin(), m_Entries.end(),
		[](const DEPLOY_RUNTIME_ENTRY& entry)
		{
			return nullptr != entry.object &&
				entry.object->Is_AnimBindPoseOnly();
		}));
	m_Status = "DeployProp committed: " +
		std::to_string(m_Catalog.Get_Assets().size()) + " assets, " +
		std::to_string(m_Entries.size()) + " placements, " +
		std::to_string(bindPoseOnly) + " bind-pose-only";
	return true;
}

void CDeployPropRuntime::Clear()
{
	if (m_iLevelIndex < ETOUI(LEVEL::END))
		Remove_Entries(m_iLevelIndex, m_Entries);
	else
		m_Entries.clear();

	m_iLevelIndex = ETOUI(LEVEL::END);
	m_EntryIndex.clear();
	m_Catalog = CDeployPropCatalog{};
	m_Status = "DeployProp runtime not loaded";
}

void CDeployPropRuntime::Reset_ClearedLevelTracking()
{
	m_iLevelIndex = ETOUI(LEVEL::END);
	m_Entries.clear();
	m_EntryIndex.clear();
	m_Catalog = CDeployPropCatalog{};
	m_Status = "DeployProp level resources were cleared";
}

bool_t CDeployPropRuntime::Set_State(
	const uint64_t runtimePlacementId,
	const DEPLOY_PROP_STATE state)
{
	const auto iter = m_EntryIndex.find(runtimePlacementId);
	if (iter == m_EntryIndex.end() || iter->second >= m_Entries.size() ||
		nullptr == m_Entries[iter->second].object)
	{
		m_Status = "DeployProp runtime placement ID is unknown";
		return false;
	}
	if (!m_Entries[iter->second].object->Set_State(state))
	{
		m_Status = "DeployProp presentation rejected the requested state";
		return false;
	}
	return true;
}

bool_t CDeployPropRuntime::Set_States(
	const std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>>& placementStates)
{
	if (placementStates.empty())
	{
		m_Status = "DeployProp state transaction is empty";
		return false;
	}

	struct STAGED_STATE final
	{
		shared_ptr<CDeployPropObject> object;
		DEPLOY_PROP_STATE previousState = DEPLOY_PROP_STATE::INTACT;
		DEPLOY_PROP_STATE targetState = DEPLOY_PROP_STATE::INTACT;
		bool_t applied = false;
	};
	std::vector<STAGED_STATE> staged;
	staged.reserve(placementStates.size());
	std::unordered_map<uint64_t, bool_t> uniqueIds;
	uniqueIds.reserve(placementStates.size());
	for (const auto& [placementId, targetState] : placementStates)
	{
		const auto iter = m_EntryIndex.find(placementId);
		if (0u == placementId || !uniqueIds.emplace(placementId, true).second ||
			iter == m_EntryIndex.end() || iter->second >= m_Entries.size() ||
			nullptr == m_Entries[iter->second].object)
		{
			m_Status = "DeployProp state transaction has an unknown member";
			return false;
		}
		const shared_ptr<CDeployPropObject>& object = m_Entries[iter->second].object;
		staged.push_back({ object, object->Get_State(), targetState, false });
	}

	for (size_t index = 0u; index < staged.size(); ++index)
	{
		staged[index].applied =
			staged[index].previousState != staged[index].targetState;
		if (staged[index].object->Set_State(staged[index].targetState))
			continue;

		bool_t rollbackSucceeded = true;
		for (size_t rollback = index + 1u; rollback-- > 0u;)
		{
			if (staged[rollback].applied)
			{
				rollbackSucceeded = staged[rollback].object->Set_State(
					staged[rollback].previousState) && rollbackSucceeded;
			}
		}
		m_Status = rollbackSucceeded ?
			"DeployProp state transaction rolled back" :
			"DeployProp state transaction rollback failed";
		return false;
	}

	m_Status = "DeployProp state transaction committed: " +
		std::to_string(staged.size()) + " placements";
	return true;
}

bool_t CDeployPropRuntime::Set_State_All(const DEPLOY_PROP_STATE state)
{
	std::vector<DEPLOY_PROP_STATE> previousStates;
	previousStates.reserve(m_Entries.size());
	for (const DEPLOY_RUNTIME_ENTRY& entry : m_Entries)
	{
		if (nullptr == entry.object)
		{
			m_Status = "DeployProp runtime contains an empty object";
			return false;
		}
		previousStates.push_back(entry.object->Get_State());
	}

	for (size_t index = 0; index < m_Entries.size(); ++index)
	{
		if (m_Entries[index].object->Set_State(state))
			continue;

		for (size_t rollback = 0; rollback < index; ++rollback)
			m_Entries[rollback].object->Set_State(previousStates[rollback]);
		m_Status = "DeployProp state transaction rolled back";
		return false;
	}
	return true;
}

shared_ptr<CDeployPropObject> CDeployPropRuntime::Find(
	const uint64_t runtimePlacementId) const
{
	const auto iter = m_EntryIndex.find(runtimePlacementId);
	return iter != m_EntryIndex.end() && iter->second < m_Entries.size() ?
		m_Entries[iter->second].object : nullptr;
}

void CDeployPropRuntime::Remove_Entries(
	const uint32_t levelIndex,
	std::vector<DEPLOY_RUNTIME_ENTRY>& entries)
{
	for (DEPLOY_RUNTIME_ENTRY& entry : entries)
	{
		if (nullptr == entry.object)
			continue;
		CGameInstance::Get().Remove_GameObject_from_Layer(
			levelIndex,
			DEPLOY_PROP_LAYER,
			static_pointer_cast<CGameObject>(entry.object));
	}
	entries.clear();
}

void CDeployPropRuntime::Move_From(CDeployPropRuntime&& other) noexcept
{
	m_iLevelIndex = std::exchange(other.m_iLevelIndex, ETOUI(LEVEL::END));
	m_Catalog = std::move(other.m_Catalog);
	m_Entries = std::move(other.m_Entries);
	m_EntryIndex = std::move(other.m_EntryIndex);
	m_Status = std::move(other.m_Status);
	other.m_Entries.clear();
	other.m_EntryIndex.clear();
	other.m_Status = "DeployProp runtime moved";
}
