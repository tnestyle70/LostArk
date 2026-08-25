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
struct BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT;

enum class VALTAN_PATTERN_EFFECT_REPEAT_POLICY : uint8_t
{
	ONCE,
	EACH_LOOP,
	END
};

enum class VALTAN_PATTERN_EFFECT_SCALE_POLICY : uint8_t
{
	OWNER_RELATIVE,
	GAMEPLAY_FOOTPRINT,
	ARENA_ABSOLUTE,
	END
};

struct VALTAN_PATTERN_EFFECT_CUE final
{
	std::string strBindingId;
	std::string strOccurrenceId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	std::string strClipOccurrenceId;
	std::string strEffectAssetId;
	/* Optional parallel Material V1 audition target.  Product timing and
	   attachment continue to come from this cue; only the Effect asset selected
	   by the Debug audition switch changes. */
	std::string strV1EffectAssetId;
	std::string strAnchorSlotId = "root";
	EFFECT_TRANSFORM_DESC LocalTransform{};
	EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	VALTAN_PATTERN_EFFECT_REPEAT_POLICY eRepeatPolicy =
		VALTAN_PATTERN_EFFECT_REPEAT_POLICY::ONCE;
	VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy =
		VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
	float3_t vWorldScale{ 1.f, 1.f, 1.f };
	bool_t bHasExplicitScalePolicy = false;
	/* v2 values are absolute source-local positions in the referenced clip
	occurrence.  The names remain source-compatible with the v1 readers. */
	uint32_t iStartMs = 0u;
	uint32_t iEndMs = 0u;
	bool_t bHasSourceEnd = false;
	bool_t bUsesLegacyStageWallTime = false;
	uint32_t iStageIndex = 0u;
	uint32_t iStageDurationMs = 0u;
};

struct VALTAN_PATTERN_EFFECT_CUE_DOCUMENT final
{
	uint32_t iFormatVersion = 1u;
	std::string strOwnerArchetypeId;
	std::vector<VALTAN_PATTERN_EFFECT_CUE> Cues;
};

/* Action- and clip-occurrence-qualified Product presentation cues for Valtan.
   The document joins each cue to the authoritative encounter tuple and to one
   stable occurrence in the action's animation binding.  Multiple actions may
   intentionally share a model clip while retaining distinct occurrence IDs.
   Every public load stages into a temporary document and only replaces the
   caller's output after the entire contract has validated. */
class CValtanPatternEffectCueDocument final
{
public:
	static std::filesystem::path Resolve_Path();
	static std::filesystem::path Resolve_V1AliasPath();
	static bool_t Parse_Text(
		std::string_view Text,
		const CEncounterPatternReference& Encounter,
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
	static bool_t Parse_Text(
		std::string_view Text,
		const CEncounterPatternReference& Encounter,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
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
