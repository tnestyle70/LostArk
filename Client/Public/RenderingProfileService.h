#pragma once

#include "Client_Defines.h"
#include "Engine_RenderTypes.h"
#include "Engine_Defines.h"
#include "DataJson.h"

#include <array>
#include <filesystem>
#include <map>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct SCENE_ENVIRONMENT_REGION final
{
    string strRegionId;
    float3_t vBoundsMinimum{}, vBoundsMaximum{};
    // Convex source brush planes: inside when dot(normal, position) + w <= 0.
    vector<float4_t> Planes;
    HEIGHT_FOG_SETTINGS Fog{};
    float4_t vDirectionalColor{};
    float4_t vAmbientColor{};
    // Omitted regions inherit the scene receiver; explicit ALL preserves a lit area.
    bool_t bHasReceiver = false;
    Engine::LIGHT_RECEIVER eReceiver = Engine::LIGHT_RECEIVER::ALL;
    bool_t bHasSourceCharacterAmbient = false;
    float4_t vSourceCharacterAmbient{};
    bool_t bHasSpecularColor = false;
    float4_t vSpecularColor{};
    f32_t fBlendTimeIn = 1.f, fBlendTimeOut = 1.f;
    f32_t fPriority = 0.f;
    // Optional full regional quality keeps an independently authored area stable.
    bool_t bHasQualityOverride = false;
    RENDER_QUALITY_SETTINGS QualityOverride{};
    bool_t bHasPostProcess = false;
    f32_t fBloomThreshold = 1.f, fBloomIntensity = 0.8f;
    float4_t vBloomTint{ 1.f, 1.f, 1.f, 1.f };
    f32_t fSceneDesaturation = 0.f;
    bool_t bHasSourcePostProcess = false;
    SOURCE_POST_PROCESS_SETTINGS SourcePostProcess{};
};

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
    vector<SCENE_ENVIRONMENT_REGION> EnvironmentRegions;
	// Empty cube ID explicitly disables scene reflection. RGBM6 decode remains
	// in the selected source program; this profile owns only scene inputs.
	string strEnvironmentCubeAssetId;
	float4_t vEnvironmentColor{ 1.f, 1.f, 1.f, 0.f };
	float4_t vEnvironmentRotationIntensity{ 0.f, 1.f, 1.f, 0.f };
	// Project Lambert SH3 integrated from the cooked RGBM6 cube, not native SH9 packing.
	// Presence preserves an explicitly disabled block through authored Save.
	bool_t bHasEnvironmentDiffuse = false;
	std::array<float4_t, 7> EnvironmentDiffuseSH{};
	f32_t fEnvironmentDiffuseIntensity = 0.f;
	bool_t bHasSourcePBRIndirect = false;
	bool_t bUseSourcePBRIndirect = false;
};

struct RENDERING_COMPARISON_OPTIONS final
{
    bool_t bActive = false;
    bool_t bDirectionalEnabled = true;
    bool_t bLutEnabled = true;
    bool_t bFXAAEnabled = true;
    bool_t bBloomEnabled = true;
    f32_t fExposureMultiplier = 1.f;
};

class CRenderingProfileService final
{
public:
	static constexpr const char_t* LOADING_PROFILE_ID =
		"scene.loading.neutral.v1";

public:
    const RENDERING_COMPARISON_OPTIONS& Get_ComparisonOptions() const { return m_ComparisonOptions; }
    bool_t Set_ComparisonOptions(const RENDERING_COMPARISON_OPTIONS& options);
    void Clear_ComparisonOptions() { m_ComparisonOptions = {}; }
	bool_t Load_Runtime(string& strOutStatus);
    // Transient presentation inputs are applied after camera regions and never saved.
    bool_t Apply_CameraEnvironment(f32_t deltaSeconds, string& status,
        bool_t suppressFog = false, const LIGHT_DESC* directionalOverride = nullptr,
        f32_t directionalBrightnessMultiplier = 1.f, const float4_t* directionalColor = nullptr);
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
	const string& Get_AppliedEnvironmentRegionId() const noexcept { return m_strAppliedEnvironmentRegion; }
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
    bool_t Apply_CameraRegionEnvironment(f32_t deltaSeconds, string& status);
    bool_t Restore_PresentationEnvironment(string& status);
    bool_t m_bPresentationFogOverride = false;
    bool_t m_bPresentationLightOverride = false;
    bool_t m_bPresentationQualityOverride = false;
    RENDER_QUALITY_SETTINGS m_PresentationBaseQuality{};
    RENDERING_COMPARISON_OPTIONS m_ComparisonOptions{};
    HEIGHT_FOG_SETTINGS m_PresentationBaseFog{};
    LIGHT_DESC m_PresentationBaseLight{};
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
    string m_strAppliedEnvironmentRegion;
    HEIGHT_FOG_SETTINGS m_EnvironmentFogFrom{};
    LIGHT_DESC m_EnvironmentLightFrom{};
    RENDER_QUALITY_SETTINGS m_EnvironmentQualityFrom{};
    f32_t m_fEnvironmentElapsed = 0.f, m_fEnvironmentDuration = 0.f;
    f32_t m_fEnvironmentExitDuration = 1.f;
	string m_strLevelQualityProfileId;
	RENDER_QUALITY_SETTINGS m_EffectiveQuality{};
};

NS_END
