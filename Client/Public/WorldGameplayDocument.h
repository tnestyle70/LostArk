#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class WORLD_PLACEMENT_KIND
{
	PLAYER_SPAWN,
	NPC,
	BOSS,
	END
};

struct WORLD_GAMEPLAY_PLACEMENT
{
	std::string placementId;
	WORLD_PLACEMENT_KIND eKind = WORLD_PLACEMENT_KIND::END;
	std::string archetypeId;
	std::string encounterId;
	float3_t position = {};
	f32_t yawDegrees = {};
	bool_t isEnabled = true;
};

class CWorldGameplayDocument final
{
public:
	static constexpr uint32_t MAX_PLACEMENT_COUNT = 4096;

	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		const std::string& areaId,
		std::string& outStatus) const;

	bool_t Add(
		const WORLD_GAMEPLAY_PLACEMENT& placement,
		std::string& outStatus);
	bool_t Remove(const std::string& placementId);
	WORLD_GAMEPLAY_PLACEMENT* Find(const std::string& placementId);
	const WORLD_GAMEPLAY_PLACEMENT* Find(
		const std::string& placementId) const;

	const std::vector<WORLD_GAMEPLAY_PLACEMENT>& Get_Placements() const
	{
		return m_Placements;
	}

	uint32_t Get_Revision() const { return m_iRevision; }
	void Mark_Edited() { ++m_iRevision; }

	static bool_t Is_Valid(
		const WORLD_GAMEPLAY_PLACEMENT& placement);
	static const char_t* Kind_ToString(WORLD_PLACEMENT_KIND kind);
	static bool_t Try_ParseKind(
		const std::string& value,
		WORLD_PLACEMENT_KIND& outKind);

private:
	std::vector<WORLD_GAMEPLAY_PLACEMENT> m_Placements;
	uint32_t m_iRevision = 1;
};

NS_END
