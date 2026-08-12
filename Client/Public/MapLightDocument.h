#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

struct MAP_POINT_LIGHT_RECORD final
{
	std::string lightId;
	std::string sourceLevel;
	std::string sourceObjectId;
	float3_t position = {};
	f32_t radiusMeters = 0.f;
	f32_t falloffExponent = 1.f;
	float4_t color = { 1.f, 1.f, 1.f, 1.f };
	f32_t brightness = 1.f;
};

/* Strict, read-only projection of an Area's imported point-light layer.
   Static map placements remain the geometry authority; this document restores
   source light actors that the static-placement extractor cannot represent. */
class CMapLightDocument final
{
public:
	static constexpr size_t MAX_LIGHT_COUNT = 64u;

public:
	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		std::string& outStatus);
	void Clear();

	bool_t Is_Ready() const { return m_isReady; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Provenance() const { return m_Provenance; }
	const std::vector<MAP_POINT_LIGHT_RECORD>& Get_Lights() const
	{
		return m_Lights;
	}

private:
	std::string m_AreaId;
	std::string m_Provenance;
	std::vector<MAP_POINT_LIGHT_RECORD> m_Lights;
	bool_t m_isReady = false;
};

NS_END
