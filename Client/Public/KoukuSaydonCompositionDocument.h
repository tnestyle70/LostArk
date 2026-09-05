#pragma once

#include "KoukuSaydonAnimationActionDocument.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE final
	{
		std::string strOccurrenceId;
		std::string strProfileId;
		// Action 0 is a valid reference; physical clips use action 0 plus stage RAW.
		std::uint32_t iSourceActionId = 0u;
		std::string strSourceStageId;
		std::string strSourceSlotId;
		std::string strReferenceRevision;
		std::string strRuntimeClip;
		std::uint32_t iStartOffsetMs = 0u;
		std::uint32_t iSourceStartMs = 0u;
		std::uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		std::string strEndPolicy;

		bool operator==(
			const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_STAGE final
	{
		std::string strStageId;
		std::string strActionId;
		std::string strStageKind;
		std::uint32_t iDurationMs = 0u;
		std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE>
			AnimationOccurrences;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_STAGE&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_PATTERN final
	{
		std::string strPatternId;
		std::string strActorProfileId;
		std::string strDisplayName;
		std::string strAuthoringStatus;
		std::string strCategory;
		std::uint32_t iNextStageOrdinal = 1u;
		std::uint32_t iNextAnimationOrdinal = 1u;
		std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> Stages;
		// A malformed Pattern remains visible and survives unrelated saves.
		// These fields are editor state; Serialize writes the preserved JSON.
		std::string strLoadError;
		std::string strPreservedJson;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_PATTERN&) const = default;
	};

	struct KOUKU_SAYDON_COMPOSITION_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 2u;
		std::uint32_t iRevision = 1u;
		std::string strCompositionId;
		std::string strEncounterId;
		std::string strBossArchetypeId;
		std::string strBossPlacementId;
		std::string strAreaId;
		std::uint32_t iFixedTickHz = 30u;
		std::uint32_t iNextPatternOrdinal = 1u;
		std::vector<std::string> PlayAllPatternIds;
		std::vector<KOUKU_SAYDON_COMPOSITION_PATTERN> Patterns;

		bool operator==(const KOUKU_SAYDON_COMPOSITION_DOCUMENT&) const = default;
	};

	struct KOUKU_SAYDON_ACTION_REFERENCE_SET final
	{
		std::array<KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT, 4u>
			Documents;
		std::array<std::string, 4u> SourceBytes;

		bool operator==(const KOUKU_SAYDON_ACTION_REFERENCE_SET&) const = default;
	};

	/* Composition edits own their file. Optional animation references assist
	   Resource browsing but never gate the Pattern list or composition Save. */
	class CKoukuSaydonCompositionDocument final
	{
	public:
		CKoukuSaydonCompositionDocument() = default;
		explicit CKoukuSaydonCompositionDocument(std::filesystem::path path);

		static std::filesystem::path Resolve_Path();
		static bool_t Is_KnownProfile(std::string_view profileId);
		// Source profile 07 shares actor 05; unknown profiles return an empty view.
		static std::string_view Resolve_ActorProfileId(std::string_view sourceProfileId);
		static bool_t Parse_Text(
			std::string_view text,
			KOUKU_SAYDON_COMPOSITION_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool_t Load_ImmutableActionReferences(
			KOUKU_SAYDON_ACTION_REFERENCE_SET& outReferences,
			std::string& outStatus);
		static bool_t Validate(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
			const KOUKU_SAYDON_ACTION_REFERENCE_SET& references,
			std::string& outStatus);
		static std::string Serialize(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document);

		bool_t Reload(std::string& outStatus);
		bool_t Reload_FromPath(
			const std::filesystem::path& path,
			std::string& outStatus);
		/* Candidate keeps the currently admitted revision. Save increments it,
		   performs source compare-and-swap, atomically replaces the
		   file, and only then commits the exact reopened document as LastGood. */
		bool_t Save_Atomic(
			const KOUKU_SAYDON_COMPOSITION_DOCUMENT& candidate,
			std::string& outStatus);

		[[nodiscard]] bool_t Has_LastGood() const noexcept {
			return m_bHasLastGood;
		}
		[[nodiscard]] bool_t Is_Fresh() const noexcept { return m_bFresh; }
		[[nodiscard]] std::uint64_t Get_Generation() const noexcept {
			return m_iGeneration;
		}
		[[nodiscard]] const std::filesystem::path& Get_Path() const noexcept {
			return m_Path;
		}
		[[nodiscard]] const KOUKU_SAYDON_COMPOSITION_DOCUMENT& Get_LastGood() const {
			return m_LastGood;
		}
		[[nodiscard]] const KOUKU_SAYDON_ACTION_REFERENCE_SET& Get_References() const {
			return m_References;
		}
		[[nodiscard]] const std::string& Get_Status() const noexcept {
			return m_strStatus;
		}

	private:
		std::filesystem::path m_Path;
		KOUKU_SAYDON_COMPOSITION_DOCUMENT m_LastGood;
		KOUKU_SAYDON_ACTION_REFERENCE_SET m_References;
		std::string m_strBaselineSourceBytes;
		std::string m_strStatus;
		std::uint64_t m_iGeneration = 0u;
		bool_t m_bHasLastGood = false;
		bool_t m_bFresh = false;
	};
}
