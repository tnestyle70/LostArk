#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class DEPLOY_PROP_MODEL_KIND
{
	STATIC,
	ANIM,
};

enum class DEPLOY_PROP_STATE
{
	INTACT,
	FRACTURED,
	DESPAWNED,
};

enum class DEPLOY_PROP_PLACEMENT_PROVENANCE
{
	SOURCE_EXACT,
	PROJECT_AUTHORED,
};

struct DEPLOY_PROP_ANIMATION_ROLES
{
	/* Empty means that entering the state changes no animation. Catalog v2
	   rows are migrated in memory to the historical on/off logical names. */
	std::string intactClip;
	std::string fracturedClip;
};

struct DEPLOY_PROP_ASSET_ENTRY
{
	std::string id;
	std::string label;
	std::string evidence;
	DEPLOY_PROP_MODEL_KIND kind = DEPLOY_PROP_MODEL_KIND::STATIC;
	std::filesystem::path intactRelativePath;
	std::filesystem::path intactResolvedPath;
	std::wstring intactPrototypeTag;
	std::filesystem::path fracturedRelativePath;
	std::filesystem::path fracturedResolvedPath;
	std::wstring fracturedPrototypeTag;
	f32_t emissiveIntensity = 1.f;
	bool_t deferredEmissiveOverlay = false;
	DEPLOY_PROP_ANIMATION_ROLES animationRoles;
	bool_t strictAnimationRoles = false;
};

struct DEPLOY_PROP_PLACEMENT
{
	uint64_t runtimePlacementId = {};
	uint32_t deployActorId = {};
	uint32_t propDefinitionId = {};
	std::string sourcePlacementId;
	std::string assetId;
	float3_t position = {};
	float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	f32_t uniformScale = 1.f;
	bool_t destructible = false;
	uint32_t stateOffActionId = {};
	uint32_t triggerBinaryOccurrenceCount = {};
	DEPLOY_PROP_PLACEMENT_PROVENANCE provenance =
		DEPLOY_PROP_PLACEMENT_PROVENANCE::SOURCE_EXACT;
};

class CDeployPropCatalog final
{
public:
	bool_t Load_Default(const std::string& expectedAreaId);
	bool_t Load(const std::filesystem::path& catalogPath,
		const std::filesystem::path& placementPath,
		const std::string& expectedAreaId);
	bool_t Save_Placements();
	bool_t Save_Placements(const std::filesystem::path& placementPath);
	bool_t Add_ProjectAuthoredPlacement(DEPLOY_PROP_PLACEMENT placement);
	bool_t Update_ProjectAuthoredPlacement(
		const DEPLOY_PROP_PLACEMENT& placement);
	bool_t Remove_ProjectAuthoredPlacement(uint64_t runtimePlacementId);

	const DEPLOY_PROP_ASSET_ENTRY* Find(const std::string& assetId) const;
	const std::vector<DEPLOY_PROP_ASSET_ENTRY>& Get_Assets() const { return m_Assets; }
	const std::vector<DEPLOY_PROP_PLACEMENT>& Get_Placements() const { return m_Placements; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Status() const { return m_Status; }
	bool_t Is_Ready() const { return m_bReady; }

private:
	std::vector<DEPLOY_PROP_ASSET_ENTRY> m_Assets;
	std::vector<DEPLOY_PROP_PLACEMENT> m_Placements;
	std::string m_AreaId;
	std::filesystem::path m_PlacementPath;
	std::string m_Status = "DeployProp catalog not loaded";
	bool_t m_bReady = false;
};

NS_END
