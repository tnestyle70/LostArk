#pragma once

#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CEncounterPatternReference;

struct VALTAN_PATTERN_EFFECT_CUE final
{
	std::string strBindingId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	std::string strEffectAssetId;
	std::string strAnchorSlotId = "root";
	EFFECT_TRANSFORM_DESC LocalTransform{};
	EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	uint32_t iStartMs = 0u;
	uint32_t iEndMs = 0u;
	uint32_t iStageIndex = 0u;
	uint32_t iStageDurationMs = 0u;
};

struct VALTAN_PATTERN_EFFECT_CUE_DOCUMENT final
{
	uint32_t iFormatVersion = 1u;
	std::string strOwnerArchetypeId;
	std::vector<VALTAN_PATTERN_EFFECT_CUE> Cues;
};

/* Action-qualified Product presentation cues for Valtan.  The document joins
   directly to the authoritative encounter tuple instead of using animation
   clip identity, because multiple Valtan actions can intentionally share one
   clip.  Every public load stages into a temporary document and only replaces
   the caller's output after the entire contract has validated. */
class CValtanPatternEffectCueDocument final
{
public:
	static std::filesystem::path Resolve_Path();
	static bool_t Parse_Text(
		std::string_view Text,
		const CEncounterPatternReference& Encounter,
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
	/* Source/Tool path.  Validates the JSON and encounter join without
	   requiring the generated runtime Effect catalog to be published yet. */
	static bool_t Load_Source(
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
	/* Product/loading path.  Adds runtime catalog and spawn-admission checks to
	   Load_Source and preserves InOutDocument on any failure. */
	static bool_t Load_ForProductPrewarm(
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
};

NS_END
