#pragma once

#include "Client_Defines.h"
#include "DeployPropCatalog.h"
#include "DeployPropObject.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

NS_BEGIN(Client)

struct DEPLOY_RUNTIME_ENTRY
{
	DEPLOY_PROP_PLACEMENT placement;
	shared_ptr<CDeployPropObject> object;
};

class CDeployPropRuntime final
{
public:
	CDeployPropRuntime() = default;
	~CDeployPropRuntime();

	CDeployPropRuntime(const CDeployPropRuntime&) = delete;
	CDeployPropRuntime& operator=(const CDeployPropRuntime&) = delete;
	CDeployPropRuntime(CDeployPropRuntime&& other) noexcept;
	CDeployPropRuntime& operator=(CDeployPropRuntime&& other) noexcept;

	bool_t Load_Area(uint32_t levelIndex, const std::string& areaId);
	bool_t Load(uint32_t levelIndex, CDeployPropCatalog catalog);
	void Clear();
	void Reset_ClearedLevelTracking();

	bool_t Set_State(uint64_t runtimePlacementId, DEPLOY_PROP_STATE state);
	bool_t Set_States(const std::vector<
		std::pair<uint64_t, DEPLOY_PROP_STATE>>& placementStates);
	using DEPLOY_SURFACE_PRESENTATION_UPDATE = std::pair<
		uint64_t, DEPLOY_SURFACE_PRESENTATION_PACKET>;
	bool_t Set_SurfacePresentations(const std::vector<
		DEPLOY_SURFACE_PRESENTATION_UPDATE>& updates);
	bool_t Get_SurfacePresentation(
		uint64_t runtimePlacementId,
		DEPLOY_SURFACE_PRESENTATION_PACKET& outPacket) const;
	bool_t Set_State_All(DEPLOY_PROP_STATE state);
	shared_ptr<CDeployPropObject> Find(uint64_t runtimePlacementId) const;

	const CDeployPropCatalog& Get_Catalog() const { return m_Catalog; }
	const std::vector<DEPLOY_RUNTIME_ENTRY>& Get_Entries() const
	{
		return m_Entries;
	}
	const std::string& Get_Status() const { return m_Status; }
	bool_t Is_Loaded() const { return m_iLevelIndex < ETOUI(LEVEL::END); }

private:
	static void Remove_Entries(
		uint32_t levelIndex,
		std::vector<DEPLOY_RUNTIME_ENTRY>& entries);
	void Move_From(CDeployPropRuntime&& other) noexcept;

private:
	uint32_t m_iLevelIndex = ETOUI(LEVEL::END);
	CDeployPropCatalog m_Catalog;
	std::vector<DEPLOY_RUNTIME_ENTRY> m_Entries;
	std::unordered_map<uint64_t, size_t> m_EntryIndex;
	std::string m_Status = "DeployProp runtime not loaded";
};

NS_END
