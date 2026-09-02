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
	/* Explicit Product timing measured from the owning Stage wall. It carries
	   no clip occurrence identity and may coexist with either NONE or a normal
	   animation binding. iStartMs stores its stageOffsetMs. */
	bool_t bUsesStageClock = false;
	uint32_t iStageIndex = 0u;
	uint32_t iStageDurationMs = 0u;
};

struct VALTAN_PATTERN_EFFECT_CUE_DOCUMENT final
{
	uint32_t iFormatVersion = 1u;
	std::string strOwnerArchetypeId;
	std::vector<VALTAN_PATTERN_EFFECT_CUE> Cues;
};

/* Product presentation cues for Valtan. Clip-driven rows join to one stable
   occurrence in the action binding; STAGE_CLOCK rows join to the owning action
   and carry no clip identity, independently of that action's animation mode.
   Every public load stages into a temporary document and only replaces the
   caller's output after full validation. */
class CValtanPatternEffectCueDocument final
{
public:
	/* Arena-centered cues never sample a Client player transform.  The authored
	   landing position and a Server-owned facing form their world anchor;
	   arena.center deliberately ignores that yaw, arena.center.facing consumes
	   the occurrence lock, and arena.center.target-follow consumes the current
	   accepted Server-tick facing. */
	static bool_t Try_BuildArenaCenterAnchor(
		std::string_view strAnchorSlotId,
		const float3_t& vArenaCenter,
		f32_t fLockedFacingYawDegrees,
		float4x4_t& OutAnchor);
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
	/* Read-only generated Product view for authoring tools. The writable owner
	   is Data/Valtan/Valtan.presentation.json; this loader never exposes a Save
	   path for Valtan.patterneffectcues.json. */
	static bool_t Load_ReadOnlyProduct(
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
	/* Product/loading path.  Adds runtime catalog and spawn-admission checks to
	   Load_ReadOnlyProduct and preserves InOutDocument on any failure. */
	static bool_t Load_ForProductPrewarm(
		VALTAN_PATTERN_EFFECT_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
};

NS_END
