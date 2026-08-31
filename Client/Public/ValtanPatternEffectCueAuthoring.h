#pragma once

#include "ValtanPatternTree.h"

#include <functional>
#include <string>

NS_BEGIN(Client)

enum class VALTAN_EFFECT_CUE_AUTHORING_ADMISSION : uint8_t
{
	DISPLAY_ONLY,
	ADMITTED,
	END
};

struct VALTAN_EFFECT_CUE_AUTHORING_CONTEXT final
{
	using SOURCE_MEMBERSHIP_QUERY = std::function<bool_t(
		const std::string&, bool_t&, std::string&)>;

	VALTAN_EFFECT_CUE_AUTHORING_ADMISSION eAdmission =
		VALTAN_EFFECT_CUE_AUTHORING_ADMISSION::DISPLAY_ONLY;
	SOURCE_MEMBERSHIP_QUERY QuerySourceMembership;
};

/* Dependency-light mutation boundary shared by CBalanceTool and the Core
   native harness. It owns no disk document and never publishes Product data:
   callers supply one admitted joined tree plus the existing Effect source
   catalog membership query, and commit dirty state only when bOutChanged is
   true. Every rejected operation preserves InOutTree byte-for-value. */
class CValtanPatternEffectCueAuthoring final
{
public:
	static bool_t Add(
		VALTAN_PATTERN_TREE_VIEW& InOutTree,
		const std::string& strPatternId,
		const std::string& strStageId,
		const std::string& strActionId,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
		const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
		bool_t& bOutChanged,
		std::string& strOutStatus);

	static bool_t Update(
		VALTAN_PATTERN_TREE_VIEW& InOutTree,
		const std::string& strPatternId,
		const std::string& strStageId,
		const std::string& strActionId,
		const std::string& strCueId,
		const std::string& strOccurrenceId,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
		const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
		bool_t& bOutChanged,
		std::string& strOutStatus);

	static bool_t Remove(
		VALTAN_PATTERN_TREE_VIEW& InOutTree,
		const std::string& strPatternId,
		const std::string& strStageId,
		const std::string& strActionId,
		const std::string& strCueId,
		const std::string& strOccurrenceId,
		const std::string& strEffectAssetId,
		const std::string& strClipOccurrenceId,
		const VALTAN_EFFECT_CUE_AUTHORING_CONTEXT& Context,
		bool_t& bOutChanged,
		std::string& strOutStatus);

	static bool_t Validate_Mirrors(
		const VALTAN_STAGE_VIEW& Stage,
		std::string& strOutStatus);
};

NS_END
