#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <map>
#include <string_view>

NS_BEGIN(Client)

struct SCENE_RENDERING_PROFILE final
{
	string strProfileId;
	LIGHT_DESC Light{};
	f32_t fExposureMultiplier = 1.f;
	f32_t fBloomIntensityMultiplier = 1.f;
	float3_t vShadowFocus{};
	f32_t fShadowDistance = 40.f;
	SHADOW_SETTINGS ShadowSettings{};
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
	bool_t Activate_Profile(
		string_view strProfileId,
		string& strOutStatus);

	const RENDER_QUALITY_SETTINGS& Get_GlobalQuality() const
	{
		return m_Catalog.GlobalQuality;
	}
	const SCENE_RENDERING_PROFILE* Get_ActiveProfile() const;
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
	bool_t Commit_Resolved(
		const SCENE_RENDERING_PROFILE& Profile,
		const RENDER_QUALITY_SETTINGS& Effective,
		string& strOutStatus);

#ifdef _DEBUG
	static string Serialize_Catalog(const CATALOG& Catalog);
#endif

private:
	CATALOG m_Catalog;
	string m_strActiveProfileId;
	RENDER_QUALITY_SETTINGS m_EffectiveQuality{};
};

NS_END
