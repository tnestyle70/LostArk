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
	BOSS_COMBAT_OBJECT,
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

struct EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER final
{
	std::string strOwnerArchetypeId;
	std::string strCombatObjectArchetypeId;
	std::string strClientVisualId;
};

using EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER_MAP =
	std::map<std::string,
		EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER, std::less<>>;

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
	std::string strCombatObjectArchetypeId;
	std::string strClientVisualId;
	std::string strEffectAssetId;
	std::filesystem::path Path;
	bool bRegistryBoundAuditionOnly = false;
	bool bAuditionSourceFreshnessValid = true;
	std::string strSourceEffectAssetId;
	std::filesystem::path SourceDocumentPath;
	std::string strSourceDocumentRawSha256;
	std::filesystem::file_time_type LastWriteTime{};
	uint64_t iFileSize = 0u;
};

struct EFFECT_DIRECT_AUTHORED_SOURCE_INDEX final
{
	std::vector<EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY> Entries;
	size_t iCatalogDirectCount = 0u;
	size_t iUnavailableCount = 0u;
	std::string strFirstUnavailable;
	size_t iOwnerJoinUnavailableCount = 0u;
	std::string strFirstOwnerJoinUnavailable;
};

class CEffectDirectAuthoredSourceIndex final
{
public:
	/* Builds the Effect Tool's saved-unified editor list from source-catalog
	   metadata. Exact source-path admission is independent from Product-owner
	   joins: an owner failure leaves eOwnerKind == END but never removes an
	   otherwise valid authored document from Entries. A catalog-level error
	   preserves InOutIndex; invalid rows are isolated, while a structurally valid
	   audition with a stale source pin remains visible but freshness-locked. */
	static bool Build(
		const std::filesystem::path& CatalogPath,
		const std::filesystem::path& AuditionCatalogPath,
		const std::filesystem::path& AuthoredRoot,
		const std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE>& ScannedFiles,
		const EFFECT_DIRECT_AUTHORED_OWNER_SET& ValidOwners,
		const EFFECT_DIRECT_AUTHORED_BOSS_OWNER_MAP& ValidBossOwners,
		const EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER_MAP&
			ValidBossCombatObjectOwners,
		EFFECT_DIRECT_AUTHORED_SOURCE_INDEX& InOutIndex,
		std::string& strOutStatus);

	/* Re-reads the catalog for an already opened registry-bound audition. This
	   prevents Play/Save from trusting an index snapshot after the exact row or
	   its ordinary Product source row was deleted, reclassified, or retargeted. */
	static bool Validate_RegistryBoundAuditionCatalogProvenanceFresh(
		const std::filesystem::path& ProductCatalogPath,
		const std::filesystem::path& AuditionCatalogPath,
		const std::filesystem::path& AuthoredRoot,
		const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Expected,
		std::string& strOutStatus);
};

}
