#pragma once

#include "Client_Defines.h"
#include "Network/PacketMessages.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
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

struct EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY final
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	LostArk::Shared::SKILL_ID iSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
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
		EFFECT_DIRECT_AUTHORED_SOURCE_INDEX& InOutIndex,
		std::string& strOutStatus);
};

}
