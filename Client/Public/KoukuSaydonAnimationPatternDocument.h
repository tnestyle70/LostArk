#pragma once

#include "KoukuSaydonAnimationActionDocument.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct KOUKU_SAYDON_ANIMATION_PATTERN_CLIP final
	{
		std::string strOccurrenceId;
		std::string strStageId;
		std::string strSlotId;
		std::string strRuntimeClip;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		std::string strEndPolicy;

		bool operator==(const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP&) const = default;
	};

	struct KOUKU_SAYDON_ANIMATION_PATTERN final
	{
		std::string strPatternId;
		std::string strDisplayName;
		std::uint32_t iSourceActionId = 0u;
		std::uint32_t iNextOccurrenceOrdinal = 1u;
		std::vector<KOUKU_SAYDON_ANIMATION_PATTERN_CLIP> Clips;

		bool operator==(const KOUKU_SAYDON_ANIMATION_PATTERN&) const = default;
	};

	/* Local animation composition data pinned to one immutable action
	reference.  REFERENCE_ONLY is the only admitted authority. */
	struct KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 1u;
		std::string strProfileId;
		std::string strReferenceRevision;
		std::string strAuthority;
		std::uint32_t iNextPatternOrdinal = 1u;
		std::vector<KOUKU_SAYDON_ANIMATION_PATTERN> Patterns;

		bool operator==(const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT&) const = default;
	};

	class CKoukuSaydonAnimationPatternDocument final
	{
	public:
		static std::filesystem::path Resolve_Path(
			std::string_view profileId);

		static bool_t Parse_Text(
			std::string_view text,
			KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
			std::string& outStatus);

		static bool_t Validate(
			const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);

		/* outDocument commits only after parse, structural validation, exact
		source identity join, and target-model clip membership all succeed. */
		static bool_t Load_FromPath(
			const std::filesystem::path& path,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Load(
			std::string_view profileId,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& outDocument,
			std::string& outStatus);

		/* Validate -> durable sibling temp -> strict reparse/exact compare ->
		reference compare-and-swap -> atomic replace. */
		static bool_t Save_Atomic(
			const KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT& document,
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
	};
}
