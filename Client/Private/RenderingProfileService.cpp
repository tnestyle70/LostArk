#include "RenderingProfileService.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <set>
#include <sstream>

namespace
{
	using namespace Client;

	constexpr size_t MAXIMUM_PROFILE_COUNT = 32u;

	filesystem::path Get_ModuleDirectory()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath,
			static_cast<DWORD>(size(modulePath)));
		return 0u == length || length >= size(modulePath) ?
			filesystem::path{} : filesystem::path(modulePath).parent_path();
	}

	filesystem::path Find_RuntimeCatalog()
	{
		const filesystem::path module = Get_ModuleDirectory();
		const filesystem::path adjacent = module / L"DataFiles" /
			L"Rendering" / L"RenderingProfiles.runtime.json";
		if (filesystem::is_regular_file(adjacent))
			return adjacent;
		const filesystem::path parent = module.parent_path() / L"DataFiles" /
			L"Rendering" / L"RenderingProfiles.runtime.json";
		return filesystem::is_regular_file(parent) ? parent : adjacent;
	}

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = object.Find(pName);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	bool_t Has_ExactFields(
		const DATA_JSON_VALUE& object,
		initializer_list<const char_t*> fields)
	{
		if (!object.Is_Object() || object.Get_Object().size() != fields.size())
			return false;
		return all_of(fields.begin(), fields.end(),
			[&object](const char_t* pField)
			{
				return nullptr != object.Find(pField);
			});
	}

	bool_t Is_StableId(const string& value)
	{
		return !value.empty() && value.size() <= 128u &&
			all_of(value.begin(), value.end(), [](const char_t character)
			{
				return (character >= 'a' && character <= 'z') ||
					(character >= 'A' && character <= 'Z') ||
					(character >= '0' && character <= '9') ||
					'.' == character || '-' == character || '_' == character;
			});
	}

	bool_t Read_Float(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		const f32_t minimum,
		const f32_t maximum,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* pValue = Required(
			object, pName, DATA_JSON_TYPE::NUMBER);
		if (nullptr == pValue || !isfinite(pValue->Get_Number()) ||
			pValue->Get_Number() < minimum || pValue->Get_Number() > maximum)
		{
			return false;
		}
		outValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_Float4(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		float4_t& outValue)
	{
		const DATA_JSON_VALUE* pValue = Required(
			object, pName, DATA_JSON_TYPE::ARRAY);
		if (nullptr == pValue || 4u != pValue->Get_Array().size())
			return false;
		f32_t values[4]{};
		for (size_t index = 0u; index < size(values); ++index)
		{
			const DATA_JSON_VALUE& item = pValue->Get_Array()[index];
			if (!item.Is_Number() || !isfinite(item.Get_Number()) ||
				item.Get_Number() < -64.0 || item.Get_Number() > 64.0)
			{
				return false;
			}
			values[index] = static_cast<f32_t>(item.Get_Number());
		}
		outValue = float4_t(values[0], values[1], values[2], values[3]);
		return true;
	}

	bool_t Read_Float3(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		float3_t& outValue)
	{
		const DATA_JSON_VALUE* pValue = Required(
			object, pName, DATA_JSON_TYPE::ARRAY);
		if (nullptr == pValue || 3u != pValue->Get_Array().size())
			return false;
		f32_t values[3]{};
		for (size_t index = 0u; index < size(values); ++index)
		{
			const DATA_JSON_VALUE& item = pValue->Get_Array()[index];
			if (!item.Is_Number() || !isfinite(item.Get_Number()) ||
				item.Get_Number() < -100000.0 ||
				item.Get_Number() > 100000.0)
			{
				return false;
			}
			values[index] = static_cast<f32_t>(item.Get_Number());
		}
		outValue = float3_t(values[0], values[1], values[2]);
		return true;
	}

	bool_t Is_FiniteRange(
		const f32_t value,
		const f32_t minimum,
		const f32_t maximum)
	{
		return isfinite(value) && value >= minimum && value <= maximum;
	}

	bool_t Is_ValidColor(const float4_t& value)
	{
		return Is_FiniteRange(value.x, 0.f, 64.f) &&
			Is_FiniteRange(value.y, 0.f, 64.f) &&
			Is_FiniteRange(value.z, 0.f, 64.f) &&
			Is_FiniteRange(value.w, 0.f, 1.f);
	}

	string Quote(const string_view value)
	{
		return "\"" + CDataJson::Escape(value) + "\"";
	}

	void Write_Float4(ostringstream& output, const float4_t& value)
	{
		output << '[' << value.x << ", " << value.y << ", " << value.z <<
			", " << value.w << ']';
	}

	void Write_Float3(ostringstream& output, const float3_t& value)
	{
		output << '[' << value.x << ", " << value.y << ", " <<
			value.z << ']';
	}

	bool_t Build_ShadowDesc(
		const SCENE_RENDERING_PROFILE& Profile,
		SHADOW_LIGHT_DESC& OutDesc)
	{
		OutDesc = {};
		OutDesc.Settings = Profile.ShadowSettings;
		if (!Profile.ShadowSettings.bEnabled)
			return true;

		const float3_t direction(
			Profile.Light.vDirection.x,
			Profile.Light.vDirection.y,
			Profile.Light.vDirection.z);
		const f32_t lengthSquared =
			direction.x * direction.x +
			direction.y * direction.y +
			direction.z * direction.z;
		if (!isfinite(lengthSquared) || lengthSquared <= 0.000001f ||
			!isfinite(Profile.fShadowDistance) ||
			Profile.fShadowDistance <= 0.f)
		{
			return false;
		}
		const f32_t inverseLength = 1.f / sqrt(lengthSquared);
		OutDesc.vAt = float4_t(
			Profile.vShadowFocus.x,
			Profile.vShadowFocus.y,
			Profile.vShadowFocus.z,
			1.f);
		OutDesc.vEye = float4_t(
			Profile.vShadowFocus.x -
				direction.x * inverseLength * Profile.fShadowDistance,
			Profile.vShadowFocus.y -
				direction.y * inverseLength * Profile.fShadowDistance,
			Profile.vShadowFocus.z -
				direction.z * inverseLength * Profile.fShadowDistance,
			1.f);
		return true;
	}
}

NS_BEGIN(Client)

bool_t CRenderingProfileService::Load_Runtime(string& strOutStatus)
{
	CATALOG staged;
	if (!Parse_Catalog(Find_RuntimeCatalog(), staged, strOutStatus))
		return false;
	m_Catalog = move(staged);
	m_strActiveProfileId.clear();
	strOutStatus = "Rendering runtime catalog loaded.";
	return true;
}

bool_t CRenderingProfileService::Reload_Runtime(string& strOutStatus)
{
	CATALOG staged;
	if (!Parse_Catalog(Find_RuntimeCatalog(), staged, strOutStatus))
		return false;
	if (m_strActiveProfileId.empty())
	{
		m_Catalog = move(staged);
		strOutStatus = "Rendering runtime catalog reloaded.";
		return true;
	}

	const auto profile = staged.Profiles.find(m_strActiveProfileId);
	if (staged.Profiles.end() == profile)
	{
		strOutStatus = "Reload rejected: active scene profile is missing.";
		return false;
	}
	RENDER_QUALITY_SETTINGS effective{};
	if (!Resolve_EffectiveQuality(
		staged.GlobalQuality, profile->second, effective, strOutStatus) ||
		!Commit_Resolved(profile->second, effective, strOutStatus))
	{
		return false;
	}
	m_Catalog = move(staged);
	m_EffectiveQuality = effective;
	strOutStatus = "Runtime catalog reloaded and active scene reapplied.";
	return true;
}

bool_t CRenderingProfileService::Has_Profile(
	const string_view strProfileId) const
{
	return m_Catalog.Profiles.end() !=
		m_Catalog.Profiles.find(strProfileId);
}

bool_t CRenderingProfileService::Activate_Profile(
	const string_view strProfileId,
	string& strOutStatus)
{
	const auto profile = m_Catalog.Profiles.find(strProfileId);
	if (m_Catalog.Profiles.end() == profile)
	{
		strOutStatus = "Unknown scene rendering profile: " + string(strProfileId);
		return false;
	}
	RENDER_QUALITY_SETTINGS effective{};
	if (!Resolve_EffectiveQuality(
		m_Catalog.GlobalQuality, profile->second, effective, strOutStatus) ||
		!Commit_Resolved(profile->second, effective, strOutStatus))
	{
		return false;
	}
	m_strActiveProfileId = profile->first;
	m_EffectiveQuality = effective;
	strOutStatus = "Activated scene rendering profile: " + profile->first;
	return true;
}

const SCENE_RENDERING_PROFILE*
CRenderingProfileService::Get_ActiveProfile() const
{
	const auto profile = m_Catalog.Profiles.find(m_strActiveProfileId);
	return m_Catalog.Profiles.end() == profile ? nullptr : &profile->second;
}

bool_t CRenderingProfileService::Apply_GlobalQuality(
	const RENDER_QUALITY_SETTINGS& Quality,
	string& strOutStatus)
{
	if (!Validate_GlobalQuality(Quality, strOutStatus))
		return false;
	const SCENE_RENDERING_PROFILE* pProfile = Get_ActiveProfile();
	if (nullptr == pProfile)
	{
		if (FAILED(CGameInstance::Get().Apply_RenderQualitySettings(Quality)))
		{
			strOutStatus = "Global rendering settings were rejected; active state preserved.";
			return false;
		}
		m_Catalog.GlobalQuality = Quality;
		m_EffectiveQuality = Quality;
		strOutStatus = "Global technical quality applied.";
		return true;
	}

	RENDER_QUALITY_SETTINGS effective{};
	if (!Resolve_EffectiveQuality(
		Quality, *pProfile, effective, strOutStatus) ||
		!Commit_Resolved(*pProfile, effective, strOutStatus))
	{
		return false;
	}
	m_Catalog.GlobalQuality = Quality;
	m_EffectiveQuality = effective;
	strOutStatus = "Global technical quality applied without scene multiplier drift.";
	return true;
}

bool_t CRenderingProfileService::Apply_ActiveProfile(
	const SCENE_RENDERING_PROFILE& Profile,
	string& strOutStatus)
{
	if (Profile.strProfileId != m_strActiveProfileId ||
		!Validate_Profile(Profile, strOutStatus))
	{
		if (Profile.strProfileId != m_strActiveProfileId)
			strOutStatus = "Active profile ID cannot be renamed by the workbench.";
		return false;
	}
	RENDER_QUALITY_SETTINGS effective{};
	if (!Resolve_EffectiveQuality(
		m_Catalog.GlobalQuality, Profile, effective, strOutStatus) ||
		!Commit_Resolved(Profile, effective, strOutStatus))
	{
		return false;
	}
	m_Catalog.Profiles[Profile.strProfileId] = Profile;
	m_EffectiveQuality = effective;
	strOutStatus = "Active scene profile applied.";
	return true;
}

bool_t CRenderingProfileService::Parse_Catalog(
	const filesystem::path& Path,
	CATALOG& OutCatalog,
	string& strOutStatus)
{
	ifstream input(Path, ios::binary);
	if (!input)
	{
		strOutStatus = "Rendering profile catalog is missing: " + Path.string();
		return false;
	}
	const string text{
		istreambuf_iterator<char>(input), istreambuf_iterator<char>() };
	DATA_JSON_VALUE root;
	string parseError;
	if (!CDataJson::Parse(text, root, parseError))
	{
		strOutStatus = "Rendering profile JSON rejected: " + parseError;
		return false;
	}
	if (!Has_ExactFields(root,
		{ "schema", "formatVersion", "revision", "globalQuality", "profiles" }))
	{
		strOutStatus = "Rendering profile root has missing or unsupported fields.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* pVersion = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pRevision = Required(
		root, "revision", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* pGlobal = Required(
		root, "globalQuality", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* pProfiles = Required(
		root, "profiles", DATA_JSON_TYPE::ARRAY);
	if (nullptr == pSchema || "lostark.rendering-profiles" != pSchema->Get_String() ||
		nullptr == pVersion || 1.0 != pVersion->Get_Number() ||
		nullptr == pRevision || !isfinite(pRevision->Get_Number()) ||
		pRevision->Get_Number() != floor(pRevision->Get_Number()) ||
		pRevision->Get_Number() < 1.0 ||
		pRevision->Get_Number() > static_cast<double>(UINT32_MAX) ||
		nullptr == pGlobal || nullptr == pProfiles ||
		pProfiles->Get_Array().empty() ||
		pProfiles->Get_Array().size() > MAXIMUM_PROFILE_COUNT)
	{
		strOutStatus = "Rendering profile schema/version/revision/count is invalid.";
		return false;
	}

	CATALOG staged;
	staged.iRevision = static_cast<uint32_t>(pRevision->Get_Number());
	if (!Has_ExactFields(*pGlobal,
		{ "ssaoEnabled", "ssaoRadius", "ssaoBias", "ssaoIntensity",
		  "ssaoPower", "ssaoDistanceFade",
		  "bloomEnabled", "bloomThreshold", "bloomSoftKnee",
		  "bloomIntensity", "bloomScatter", "exposure", "whitePoint",
		  "gamma", "fxaaEnabled", "fxaaSubpixel", "fxaaEdgeThreshold",
		  "fxaaEdgeThresholdMin" }))
	{
		strOutStatus = "globalQuality has missing or unsupported fields.";
		return false;
	}
	const DATA_JSON_VALUE* pSSAOEnabled = Required(
		*pGlobal, "ssaoEnabled", DATA_JSON_TYPE::BOOLEAN);
	const DATA_JSON_VALUE* pBloomEnabled = Required(
		*pGlobal, "bloomEnabled", DATA_JSON_TYPE::BOOLEAN);
	const DATA_JSON_VALUE* pFXAAEnabled = Required(
		*pGlobal, "fxaaEnabled", DATA_JSON_TYPE::BOOLEAN);
	if (nullptr == pSSAOEnabled || nullptr == pBloomEnabled ||
		nullptr == pFXAAEnabled ||
		!Read_Float(*pGlobal, "ssaoRadius", 0.01f, 8.f,
			staged.GlobalQuality.fSSAORadius) ||
		!Read_Float(*pGlobal, "ssaoBias", 0.f, 1.f,
			staged.GlobalQuality.fSSAOBias) ||
		!Read_Float(*pGlobal, "ssaoIntensity", 0.f, 4.f,
			staged.GlobalQuality.fSSAOIntensity) ||
		!Read_Float(*pGlobal, "ssaoPower", 0.1f, 8.f,
			staged.GlobalQuality.fSSAOPower) ||
		!Read_Float(*pGlobal, "ssaoDistanceFade", 1.f, 1000.f,
			staged.GlobalQuality.fSSAODistanceFade) ||
		!Read_Float(*pGlobal, "bloomThreshold", 0.f, 64.f,
			staged.GlobalQuality.fBloomThreshold) ||
		!Read_Float(*pGlobal, "bloomSoftKnee", 0.f, 1.f,
			staged.GlobalQuality.fBloomSoftKnee) ||
		!Read_Float(*pGlobal, "bloomIntensity", 0.f, 16.f,
			staged.GlobalQuality.fBloomIntensity) ||
		!Read_Float(*pGlobal, "bloomScatter", 0.25f, 4.f,
			staged.GlobalQuality.fBloomScatter) ||
		!Read_Float(*pGlobal, "exposure", 0.01f, 32.f,
			staged.GlobalQuality.fExposure) ||
		!Read_Float(*pGlobal, "whitePoint", 1.f, 64.f,
			staged.GlobalQuality.fWhitePoint) ||
		!Read_Float(*pGlobal, "gamma", 1.f, 3.f,
			staged.GlobalQuality.fGamma) ||
		!Read_Float(*pGlobal, "fxaaSubpixel", 0.f, 1.f,
			staged.GlobalQuality.fFXAASubpixel) ||
		!Read_Float(*pGlobal, "fxaaEdgeThreshold", 0.0312f, 0.333f,
			staged.GlobalQuality.fFXAAEdgeThreshold) ||
		!Read_Float(*pGlobal, "fxaaEdgeThresholdMin", 0.0156f, 0.0833f,
			staged.GlobalQuality.fFXAAEdgeThresholdMin))
	{
		strOutStatus = "globalQuality contains an invalid value.";
		return false;
	}
	staged.GlobalQuality.bSSAOEnabled = pSSAOEnabled->Get_Boolean();
	staged.GlobalQuality.bBloomEnabled = pBloomEnabled->Get_Boolean();
	staged.GlobalQuality.bFXAAEnabled = pFXAAEnabled->Get_Boolean();
	if (!Validate_GlobalQuality(staged.GlobalQuality, strOutStatus))
		return false;

	for (const DATA_JSON_VALUE& value : pProfiles->Get_Array())
	{
		if (!Has_ExactFields(value,
			{ "profileId", "exposureMultiplier",
			  "bloomIntensityMultiplier", "light", "shadow" }))
		{
			strOutStatus = "Scene profile has missing or unsupported fields.";
			return false;
		}
		const DATA_JSON_VALUE* pId = Required(
			value, "profileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pLight = Required(
			value, "light", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pShadow = Required(
			value, "shadow", DATA_JSON_TYPE::OBJECT);
		SCENE_RENDERING_PROFILE profile;
		if (nullptr == pId || nullptr == pLight || nullptr == pShadow ||
			!Has_ExactFields(*pLight,
				{ "type", "direction", "diffuse", "ambient", "specular" }) ||
			!Has_ExactFields(*pShadow,
				{ "enabled", "focus", "distance", "orthographicWidth",
				  "orthographicHeight", "near", "far", "depthBias",
				  "normalBias", "strength" }))
		{
			strOutStatus = "Scene profile light contract is invalid.";
			return false;
		}
		profile.strProfileId = pId->Get_String();
		const DATA_JSON_VALUE* pType = Required(
			*pLight, "type", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pShadowEnabled = Required(
			*pShadow, "enabled", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pType || "directional" != pType->Get_String() ||
			nullptr == pShadowEnabled ||
			!Read_Float(value, "exposureMultiplier", 0.1f, 4.f,
				profile.fExposureMultiplier) ||
			!Read_Float(value, "bloomIntensityMultiplier", 0.f, 4.f,
				profile.fBloomIntensityMultiplier) ||
			!Read_Float4(*pLight, "direction", profile.Light.vDirection) ||
			!Read_Float4(*pLight, "diffuse", profile.Light.vDiffuse) ||
			!Read_Float4(*pLight, "ambient", profile.Light.vAmbient) ||
			!Read_Float4(*pLight, "specular", profile.Light.vSpecular) ||
			!Read_Float3(*pShadow, "focus", profile.vShadowFocus) ||
			!Read_Float(*pShadow, "distance", 0.1f, 100000.f,
				profile.fShadowDistance) ||
			!Read_Float(*pShadow, "orthographicWidth", 0.1f, 10000.f,
				profile.ShadowSettings.fOrthographicWidth) ||
			!Read_Float(*pShadow, "orthographicHeight", 0.1f, 10000.f,
				profile.ShadowSettings.fOrthographicHeight) ||
			!Read_Float(*pShadow, "near", 0.0001f, 100000.f,
				profile.ShadowSettings.fNear) ||
			!Read_Float(*pShadow, "far", 0.0001f, 100000.f,
				profile.ShadowSettings.fFar) ||
			!Read_Float(*pShadow, "depthBias", 0.f, 0.05f,
				profile.ShadowSettings.fDepthBias) ||
			!Read_Float(*pShadow, "normalBias", 0.f, 10.f,
				profile.ShadowSettings.fNormalBias) ||
			!Read_Float(*pShadow, "strength", 0.f, 1.f,
				profile.ShadowSettings.fStrength))
		{
			strOutStatus = "Scene profile contains an invalid light or multiplier.";
			return false;
		}
		profile.Light.eType = LIGHT::DIRECTIONAL;
		profile.Light.fRange = 1.f;
		profile.ShadowSettings.bEnabled =
			pShadowEnabled->Get_Boolean();
		if (!Validate_Profile(profile, strOutStatus))
		{
			return false;
		}
		RENDER_QUALITY_SETTINGS effective{};
		if (!Resolve_EffectiveQuality(
			staged.GlobalQuality, profile, effective, strOutStatus))
		{
			return false;
		}
		if (!staged.Profiles.emplace(profile.strProfileId, profile).second)
		{
			strOutStatus = "Duplicate scene profile ID: " + profile.strProfileId;
			return false;
		}
	}
	OutCatalog = move(staged);
	return true;
}

bool_t CRenderingProfileService::Validate_GlobalQuality(
	const RENDER_QUALITY_SETTINGS& Quality,
	string& strOutStatus)
{
	const bool_t valid =
		Is_FiniteRange(Quality.fSSAORadius, 0.01f, 8.f) &&
		Is_FiniteRange(Quality.fSSAOBias, 0.f, 1.f) &&
		Is_FiniteRange(Quality.fSSAOIntensity, 0.f, 4.f) &&
		Is_FiniteRange(Quality.fSSAOPower, 0.1f, 8.f) &&
		Is_FiniteRange(Quality.fSSAODistanceFade, 1.f, 1000.f) &&
		Quality.fSSAOBias < Quality.fSSAORadius &&
		Quality.fSSAODistanceFade >= Quality.fSSAORadius &&
		Is_FiniteRange(Quality.fBloomThreshold, 0.f, 64.f) &&
		Is_FiniteRange(Quality.fBloomSoftKnee, 0.f, 1.f) &&
		Is_FiniteRange(Quality.fBloomIntensity, 0.f, 16.f) &&
		Is_FiniteRange(Quality.fBloomScatter, 0.25f, 4.f) &&
		Is_FiniteRange(Quality.fExposure, 0.01f, 32.f) &&
		Is_FiniteRange(Quality.fWhitePoint, 1.f, 64.f) &&
		Is_FiniteRange(Quality.fGamma, 1.f, 3.f) &&
		Is_FiniteRange(Quality.fFXAASubpixel, 0.f, 1.f) &&
		Is_FiniteRange(Quality.fFXAAEdgeThreshold, 0.0312f, 0.333f) &&
		Is_FiniteRange(Quality.fFXAAEdgeThresholdMin, 0.0156f, 0.0833f);
	if (!valid)
		strOutStatus = "Global technical quality is non-finite or out of range.";
	return valid;
}

bool_t CRenderingProfileService::Validate_Profile(
	const SCENE_RENDERING_PROFILE& Profile,
	string& strOutStatus)
{
	const float4_t& direction = Profile.Light.vDirection;
	const SHADOW_SETTINGS& shadow = Profile.ShadowSettings;
	const bool_t valid = Is_StableId(Profile.strProfileId) &&
		LIGHT::DIRECTIONAL == Profile.Light.eType &&
		Is_FiniteRange(direction.x, -64.f, 64.f) &&
		Is_FiniteRange(direction.y, -64.f, 64.f) &&
		Is_FiniteRange(direction.z, -64.f, 64.f) &&
		direction.x * direction.x + direction.y * direction.y +
			direction.z * direction.z > 0.000001f &&
		Is_ValidColor(Profile.Light.vDiffuse) &&
		Is_ValidColor(Profile.Light.vAmbient) &&
		Is_ValidColor(Profile.Light.vSpecular) &&
		Is_FiniteRange(Profile.fExposureMultiplier, 0.1f, 4.f) &&
		Is_FiniteRange(Profile.fBloomIntensityMultiplier, 0.f, 4.f) &&
		Is_FiniteRange(Profile.vShadowFocus.x, -100000.f, 100000.f) &&
		Is_FiniteRange(Profile.vShadowFocus.y, -100000.f, 100000.f) &&
		Is_FiniteRange(Profile.vShadowFocus.z, -100000.f, 100000.f) &&
		Is_FiniteRange(Profile.fShadowDistance, 0.1f, 100000.f) &&
		Is_FiniteRange(shadow.fOrthographicWidth, 0.1f, 10000.f) &&
		Is_FiniteRange(shadow.fOrthographicHeight, 0.1f, 10000.f) &&
		Is_FiniteRange(shadow.fNear, 0.0001f, 100000.f) &&
		Is_FiniteRange(shadow.fFar, 0.0001f, 100000.f) &&
		shadow.fFar > shadow.fNear &&
		Is_FiniteRange(shadow.fDepthBias, 0.f, 0.05f) &&
		Is_FiniteRange(shadow.fNormalBias, 0.f, 10.f) &&
		Is_FiniteRange(shadow.fStrength, 0.f, 1.f);
	if (!valid)
		strOutStatus = "Scene profile ID/light/multiplier/shadow is invalid.";
	return valid;
}

bool_t CRenderingProfileService::Resolve_EffectiveQuality(
	const RENDER_QUALITY_SETTINGS& GlobalQuality,
	const SCENE_RENDERING_PROFILE& Profile,
	RENDER_QUALITY_SETTINGS& OutEffective,
	string& strOutStatus)
{
	if (!Validate_GlobalQuality(GlobalQuality, strOutStatus) ||
		!Validate_Profile(Profile, strOutStatus))
	{
		return false;
	}
	OutEffective = GlobalQuality;
	OutEffective.fExposure =
		GlobalQuality.fExposure * Profile.fExposureMultiplier;
	OutEffective.fBloomIntensity =
		GlobalQuality.fBloomIntensity * Profile.fBloomIntensityMultiplier;
	if (!Validate_GlobalQuality(OutEffective, strOutStatus))
	{
		strOutStatus = "Global quality multiplied by the scene profile is out of range.";
		return false;
	}
	return true;
}

bool_t CRenderingProfileService::Commit_Resolved(
	const SCENE_RENDERING_PROFILE& Profile,
	const RENDER_QUALITY_SETTINGS& Effective,
	string& strOutStatus)
{
	const RENDER_QUALITY_SETTINGS previous =
		CGameInstance::Get().Get_RenderQualitySettings();
	const SHADOW_LIGHT_DESC previousShadow =
		CGameInstance::Get().Get_ShadowLightDesc();
	SHADOW_LIGHT_DESC stagedShadow{};
	if (!Build_ShadowDesc(Profile, stagedShadow))
	{
		strOutStatus = "Scene shadow direction could not be resolved.";
		return false;
	}
	if (FAILED(CGameInstance::Get().Apply_RenderQualitySettings(Effective)))
	{
		strOutStatus = "Renderer rejected the staged effective quality; active state preserved.";
		return false;
	}
	if (FAILED(CGameInstance::Get().Apply_Shadow_Light(stagedShadow)))
	{
		CGameInstance::Get().Apply_RenderQualitySettings(previous);
		strOutStatus = "Shadow service rejected the staged scene shadow; active state preserved.";
		return false;
	}
	if (FAILED(CGameInstance::Get().Add_Light(Profile.Light)))
	{
		CGameInstance::Get().Apply_Shadow_Light(previousShadow);
		CGameInstance::Get().Apply_RenderQualitySettings(previous);
		strOutStatus = "Light manager rejected the staged scene light; active state preserved.";
		return false;
	}
	return true;
}

#ifdef _DEBUG
string CRenderingProfileService::Serialize_Catalog(const CATALOG& Catalog)
{
	ostringstream output;
	output << setprecision(9);
	output << "{\n"
		"  \"schema\": \"lostark.rendering-profiles\",\n"
		"  \"formatVersion\": 1,\n"
		"  \"revision\": " << Catalog.iRevision << ",\n"
		"  \"globalQuality\": {\n"
		"    \"ssaoEnabled\": " <<
			(Catalog.GlobalQuality.bSSAOEnabled ? "true" : "false") << ",\n"
		"    \"ssaoRadius\": " << Catalog.GlobalQuality.fSSAORadius << ",\n"
		"    \"ssaoBias\": " << Catalog.GlobalQuality.fSSAOBias << ",\n"
		"    \"ssaoIntensity\": " << Catalog.GlobalQuality.fSSAOIntensity << ",\n"
		"    \"ssaoPower\": " << Catalog.GlobalQuality.fSSAOPower << ",\n"
		"    \"ssaoDistanceFade\": " <<
			Catalog.GlobalQuality.fSSAODistanceFade << ",\n"
		"    \"bloomEnabled\": " <<
			(Catalog.GlobalQuality.bBloomEnabled ? "true" : "false") << ",\n"
		"    \"bloomThreshold\": " << Catalog.GlobalQuality.fBloomThreshold << ",\n"
		"    \"bloomSoftKnee\": " << Catalog.GlobalQuality.fBloomSoftKnee << ",\n"
		"    \"bloomIntensity\": " << Catalog.GlobalQuality.fBloomIntensity << ",\n"
		"    \"bloomScatter\": " << Catalog.GlobalQuality.fBloomScatter << ",\n"
		"    \"exposure\": " << Catalog.GlobalQuality.fExposure << ",\n"
		"    \"whitePoint\": " << Catalog.GlobalQuality.fWhitePoint << ",\n"
		"    \"gamma\": " << Catalog.GlobalQuality.fGamma << ",\n"
		"    \"fxaaEnabled\": " <<
			(Catalog.GlobalQuality.bFXAAEnabled ? "true" : "false") << ",\n"
		"    \"fxaaSubpixel\": " << Catalog.GlobalQuality.fFXAASubpixel << ",\n"
		"    \"fxaaEdgeThreshold\": " << Catalog.GlobalQuality.fFXAAEdgeThreshold << ",\n"
		"    \"fxaaEdgeThresholdMin\": " << Catalog.GlobalQuality.fFXAAEdgeThresholdMin << "\n"
		"  },\n"
		"  \"profiles\": [\n";
	size_t index = 0u;
	for (const auto& [profileId, profile] : Catalog.Profiles)
	{
		output << "    {\n"
			"      \"profileId\": " << Quote(profileId) << ",\n"
			"      \"exposureMultiplier\": " << profile.fExposureMultiplier << ",\n"
			"      \"bloomIntensityMultiplier\": " <<
			profile.fBloomIntensityMultiplier << ",\n"
			"      \"light\": {\n"
			"        \"type\": \"directional\",\n"
			"        \"direction\": ";
		Write_Float4(output, profile.Light.vDirection);
		output << ",\n        \"diffuse\": ";
		Write_Float4(output, profile.Light.vDiffuse);
		output << ",\n        \"ambient\": ";
		Write_Float4(output, profile.Light.vAmbient);
		output << ",\n        \"specular\": ";
		Write_Float4(output, profile.Light.vSpecular);
		output << "\n      },\n"
			"      \"shadow\": {\n"
			"        \"enabled\": " <<
			(profile.ShadowSettings.bEnabled ? "true" : "false") << ",\n"
			"        \"focus\": ";
		Write_Float3(output, profile.vShadowFocus);
		output << ",\n"
			"        \"distance\": " << profile.fShadowDistance << ",\n"
			"        \"orthographicWidth\": " <<
			profile.ShadowSettings.fOrthographicWidth << ",\n"
			"        \"orthographicHeight\": " <<
			profile.ShadowSettings.fOrthographicHeight << ",\n"
			"        \"near\": " << profile.ShadowSettings.fNear << ",\n"
			"        \"far\": " << profile.ShadowSettings.fFar << ",\n"
			"        \"depthBias\": " <<
			profile.ShadowSettings.fDepthBias << ",\n"
			"        \"normalBias\": " <<
			profile.ShadowSettings.fNormalBias << ",\n"
			"        \"strength\": " <<
			profile.ShadowSettings.fStrength << "\n"
			"      }\n    }" <<
			(++index == Catalog.Profiles.size() ? "\n" : ",\n");
	}
	output << "  ]\n}\n";
	return output.str();
}

bool_t CRenderingProfileService::Save_Authored(string& strOutStatus)
{
	CATALOG staged = m_Catalog;
	if (UINT32_MAX == staged.iRevision)
	{
		strOutStatus = "Rendering profile revision cannot be incremented.";
		return false;
	}
	++staged.iRevision;
	const filesystem::path path = CProjectDataRoot::Resolve(
		L"Rendering/Authored/RenderingProfiles.json");
	if (path.empty())
	{
		strOutStatus = "Authored rendering profile path is outside Data.";
		return false;
	}
	error_code error;
	filesystem::create_directories(path.parent_path(), error);
	if (error)
	{
		strOutStatus = "Could not create the authored rendering directory.";
		return false;
	}
	filesystem::path temporary = path;
	temporary += L".tmp";
	{
		ofstream output(temporary, ios::binary | ios::trunc);
		const string document = Serialize_Catalog(staged);
		output.write(document.data(), static_cast<streamsize>(document.size()));
		if (!output)
		{
			strOutStatus = "Could not write the staged authored rendering profile.";
			return false;
		}
	}
	CATALOG roundTrip;
	if (!Parse_Catalog(temporary, roundTrip, strOutStatus))
	{
		filesystem::remove(temporary, error);
		return false;
	}
	if (!MoveFileExW(
		temporary.c_str(), path.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		filesystem::remove(temporary, error);
		strOutStatus = "Could not atomically promote the authored rendering profile.";
		return false;
	}
	m_Catalog.iRevision = staged.iRevision;
	strOutStatus = "Authored rendering profiles saved at revision " +
		to_string(staged.iRevision) + ".";
	return true;
}

bool_t CRenderingProfileService::Publish_Runtime(string& strOutStatus) const
{
	const filesystem::path projectRoot = CProjectDataRoot::Get().parent_path();
	const filesystem::path script = projectRoot / L"Tools" /
		L"RenderingPipeline" / L"Publish-RenderingProfiles.ps1";
	if (!filesystem::is_regular_file(script))
	{
		strOutStatus = "Rendering profile publisher is missing.";
		return false;
	}
	wstring command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
		script.wstring() + L"\" -Mode Publish";
	vector<wchar_t> mutableCommand(command.begin(), command.end());
	mutableCommand.push_back(L'\0');
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	if (!CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE,
		CREATE_NO_WINDOW, nullptr, projectRoot.c_str(), &startup, &process))
	{
		strOutStatus = "Could not start the rendering profile publisher.";
		return false;
	}
	CloseHandle(process.hThread);
	const DWORD wait = WaitForSingleObject(process.hProcess, 120000u);
	if (WAIT_TIMEOUT == wait)
	{
		TerminateProcess(process.hProcess, 124u);
		WaitForSingleObject(process.hProcess, 5000u);
		CloseHandle(process.hProcess);
		strOutStatus = "Rendering publisher timed out and its owned process was terminated.";
		return false;
	}
	DWORD exitCode = 1u;
	const bool_t succeeded = WAIT_OBJECT_0 == wait &&
		GetExitCodeProcess(process.hProcess, &exitCode) && 0u == exitCode;
	CloseHandle(process.hProcess);
	strOutStatus = succeeded ? "Rendering runtime profile published." :
		"Rendering publisher failed with exit code " + to_string(exitCode) + ".";
	return succeeded;
}
#endif

NS_END
