#pragma once

#include "Client_Defines.h"
#include "Network/PacketMessages.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Client
{

struct EFFECT_DIRECT_AUTHORED_SCANNED_FILE final
{
	std::string strEffectAssetId;
	std::filesystem::path Path;
};

using EFFECT_DIRECT_AUTHORED_OWNER_SET = std::set<std::pair<
	LostArk::Shared::CHARACTER_CLASS_ID, LostArk::Shared::SKILL_ID>>;

enum class EFFECT_DIRECT_AUTHORED_OWNER_KIND : uint8_t
{
	PLAYER_SKILL,
	BOSS_PATTERN,
	END
};

struct EFFECT_DIRECT_AUTHORED_BOSS_OWNER final
{
	std::string strOwnerArchetypeId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
};

using EFFECT_DIRECT_AUTHORED_BOSS_OWNER_MAP = std::map<std::string,
	EFFECT_DIRECT_AUTHORED_BOSS_OWNER, std::less<>>;

struct EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY final
{
	EFFECT_DIRECT_AUTHORED_OWNER_KIND eOwnerKind =
		EFFECT_DIRECT_AUTHORED_OWNER_KIND::END;
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	LostArk::Shared::SKILL_ID iSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	std::string strOwnerArchetypeId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	std::string strEffectAssetId;
	std::filesystem::path Path;
	std::filesystem::file_time_type LastWriteTime{};
	uint64_t iFileSize = 0u;
};

struct EFFECT_DIRECT_AUTHORED_SOURCE_INDEX final
{
	std::vector<EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY> Entries;
	size_t iCatalogDirectCount = 0u;
	size_t iUnavailableCount = 0u;
	std::string strFirstUnavailable;
};

class CEffectDirectAuthoredSourceIndex final
{
public:
	/* Builds the Effect Tool's saved-unified list from source-catalog metadata.
	   A catalog-level error preserves InOutIndex. Invalid individual source rows
	   are isolated and reported in the committed index. */
	static bool Build(
		const std::filesystem::path& CatalogPath,
		const std::filesystem::path& AuthoredRoot,
		const std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE>& ScannedFiles,
		const EFFECT_DIRECT_AUTHORED_OWNER_SET& ValidOwners,
		const EFFECT_DIRECT_AUTHORED_BOSS_OWNER_MAP& ValidBossOwners,
		EFFECT_DIRECT_AUTHORED_SOURCE_INDEX& InOutIndex,
		std::string& strOutStatus);
};

}
