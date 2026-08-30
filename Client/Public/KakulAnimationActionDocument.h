#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct KAKUL_ANIMATION_ACTION_SLOT_REFERENCE final
	{
		std::string strSlotId;
		std::string strExtractedClip;
		std::string strRuntimeClip;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		bool_t bLoop = false;
		std::string strMappingBasis;
		std::string strAuthority;

		bool operator==(
			const KAKUL_ANIMATION_ACTION_SLOT_REFERENCE&) const = default;
	};

	struct KAKUL_ANIMATION_ACTION_STAGE_REFERENCE final
	{
		std::string strStageId;
		std::uint32_t iStageOrdinal = 0u;
		std::vector<std::string> HoldoutClipNames;
		std::vector<KAKUL_ANIMATION_ACTION_SLOT_REFERENCE> Slots;

		bool operator==(
			const KAKUL_ANIMATION_ACTION_STAGE_REFERENCE&) const = default;
	};

	struct KAKUL_ANIMATION_ACTION_REFERENCE final
	{
		std::uint32_t iSourceActionId = 0u;
		std::string strDisplayName;
		std::string strReviewStatus;
		std::string strAuthority;
		std::vector<KAKUL_ANIMATION_ACTION_STAGE_REFERENCE> Stages;

		bool operator==(
			const KAKUL_ANIMATION_ACTION_REFERENCE&) const = default;
	};

	struct KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 1u;
		std::string strProfileId;
		std::string strModelAssetId;
		std::string strSourceEvidenceSha256;
		std::string strReferenceRevision;
		std::string strAuthority;
		std::vector<KAKUL_ANIMATION_ACTION_REFERENCE> Actions;

		bool operator==(
			const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT&) const = default;
	};

	/* Sparse local override of one exact reference slot.  It remains
	REFERENCE_ONLY and cannot create a Server Product Kakul pattern. */
	struct KAKUL_ANIMATION_ACTION_BINDING final
	{
		std::uint32_t iSourceActionId = 0u;
		std::string strStageId;
		std::string strSlotId;
		std::string strRuntimeClip;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		bool_t bLoop = false;
		std::string strMappingBasis;
		std::string strAuthority;

		bool operator==(
			const KAKUL_ANIMATION_ACTION_BINDING&) const = default;
	};

	struct KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 1u;
		std::string strProfileId;
		std::string strReferenceRevision;
		std::string strAuthority;
		std::vector<KAKUL_ANIMATION_ACTION_BINDING> Bindings;

		bool operator==(
			const KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT&) const = default;
	};

	class CKakulAnimationActionDocument final
	{
	public:
		static std::filesystem::path Resolve_ReferencePath(
			std::string_view profileId);
		static std::filesystem::path Resolve_AuthoredPath(
			std::string_view profileId);

		static bool_t Parse_ReferenceText(
			std::string_view text,
			KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Parse_AuthoredText(
			std::string_view text,
			KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT& outDocument,
			std::string& outStatus);

		static bool_t Validate_Reference(
			const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& document,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
		static bool_t Validate_Authored(
			const KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT& document,
			const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);

		/* Both outputs commit only after reference, authored, and their exact
		stable-identity join have validated. */
		static bool_t Load_FromPaths(
			const std::filesystem::path& referencePath,
			const std::filesystem::path& authoredPath,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
			KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT& outAuthored,
			std::string& outStatus);
		static bool_t Load(
			std::string_view profileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& outReference,
			KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT& outAuthored,
			std::string& outStatus);

		/* Validate -> durable sibling temp -> strict reparse/exact compare ->
		atomic replace.  The immutable reference and destination survive every
		failure. */
		static bool_t Save_Atomic(
			const KAKUL_ANIMATION_ACTION_AUTHORED_DOCUMENT& document,
			const KAKUL_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
			std::string_view expectedProfileId,
			std::string_view expectedModelAssetId,
			const std::vector<std::string>& availableClips,
			std::string& outStatus);
	};
}
