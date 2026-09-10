#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "DataJson.h"

#include <filesystem>
#include <map>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct SCENE_RENDERING_PROFILE final
{
	string strProfileId;
	string strDisplayName;
	const string& Get_DisplayName() const { return strDisplayName.empty() ? strProfileId : strDisplayName; }
	bool_t bHasQualityOverride = false;
	RENDER_QUALITY_SETTINGS QualityOverride{};
	LIGHT_DESC Light{};
	f32_t fExposureMultiplier = 1.f;
	f32_t fBloomIntensityMultiplier = 1.f;
	/* Persistent Area lights only; PLAYER/BOSS pattern lights retain their authored intensity. */
	f32_t fMapLightIntensityMultiplier = 1.f;
	float3_t vShadowFocus{};
	f32_t fShadowDistance = 40.f;
	SHADOW_SETTINGS ShadowSettings{};
	/* Fog belongs to the scene profile because it is a per Level mood value
	   that the F1 tool already saves, publishes and reloads. */
	HEIGHT_FOG_SETTINGS Fog{};
	// Empty cube ID explicitly disables scene reflection. RGBM6 decode remains
	// in the selected source program; this profile owns only scene inputs.
	string strEnvironmentCubeAssetId;
	float4_t vEnvironmentColor{ 1.f, 1.f, 1.f, 0.f };
	float4_t vEnvironmentRotationIntensity{ 0.f, 1.f, 1.f, 0.f };
};

class CRenderingProfileService final
{
public:
	static constexpr const char_t* LOADING_PROFILE_ID =
		"scene.loading.neutral.v1";

public:
	bool_t Load_Runtime(string& strOutStatus);
	bool_t Reload_Runtime(string& strOutStatus);
	bool_t Has_Profile(string_view strProfileId) const;
	std::vector<std::string> Collect_ProfileIds() const
	{
		std::vector<std::string> ids;
		for (const auto& [id, profile] : m_Catalog.Profiles) ids.push_back(id);
		return ids;
	}
	bool_t Activate_LevelProfile(string_view strProfileId, string& strOutStatus);
	bool_t Activate_Profile(
		string_view strProfileId,
		string& strOutStatus);

	const RENDER_QUALITY_SETTINGS& Get_GlobalQuality() const
	{
		return m_Catalog.GlobalQuality;
	}
	const SCENE_RENDERING_PROFILE* Get_ActiveProfile() const;
	const SCENE_RENDERING_PROFILE* Find_Profile(string_view id) const;
	const RENDER_QUALITY_SETTINGS& Get_ProfileQuality(string_view id) const;
	bool_t Update_Profile(const SCENE_RENDERING_PROFILE& profile, string& status);
	bool_t Duplicate_Profile(string_view sourceId, string_view newId, string& status, string_view displayName = {});
	bool_t Delete_Profile(string_view id, string& status);
	void Protect_ProfileIds(const vector<string>& ids);
	const string& Get_LevelQualityProfileId() const { return m_strLevelQualityProfileId; }
	const string& Get_ActiveProfileId() const
	{
		return m_strActiveProfileId;
	}

	bool_t Apply_GlobalQuality(
		const RENDER_QUALITY_SETTINGS& Quality,
		string& strOutStatus);
	bool_t Apply_ActiveProfile(
		const SCENE_RENDERING_PROFILE& Profile,
		string& strOutStatus);

#ifdef _DEBUG
	bool_t Save_Authored(string& strOutStatus);
	bool_t Publish_Runtime(string& strOutStatus) const;
#endif

private:
	struct CATALOG final
	{
		uint32_t iRevision = 1u;
		RENDER_QUALITY_SETTINGS GlobalQuality{};
		map<string, SCENE_RENDERING_PROFILE, less<>> Profiles;
	};

private:
	static bool_t Parse_Catalog(
		const filesystem::path& Path,
		CATALOG& OutCatalog,
		string& strOutStatus);
	static bool_t Parse_Quality(const DATA_JSON_VALUE& value, RENDER_QUALITY_SETTINGS& quality, string& status);
	static bool_t Validate_GlobalQuality(
		const RENDER_QUALITY_SETTINGS& Quality,
		string& strOutStatus);
	static bool_t Validate_Profile(
		const SCENE_RENDERING_PROFILE& Profile,
		string& strOutStatus);
	static bool_t Resolve_EffectiveQuality(
		const RENDER_QUALITY_SETTINGS& GlobalQuality,
		const SCENE_RENDERING_PROFILE& Profile,
		RENDER_QUALITY_SETTINGS& OutEffective,
		string& strOutStatus);
	const RENDER_QUALITY_SETTINGS& Get_ActiveLevelQuality() const;
	bool_t Commit_Resolved(
		const SCENE_RENDERING_PROFILE& Profile,
		const RENDER_QUALITY_SETTINGS& Effective,
		string& strOutStatus,
		bool_t forceReloadEnvironment = false);

#ifdef _DEBUG
	static string Serialize_Catalog(const CATALOG& Catalog);
#endif

private:
	CATALOG m_Catalog;
	vector<string> m_ProtectedProfileIds;
	string m_strActiveProfileId;
	string m_strLevelQualityProfileId;
	RENDER_QUALITY_SETTINGS m_EffectiveQuality{};
};

NS_END
