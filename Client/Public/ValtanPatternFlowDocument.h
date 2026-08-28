#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct VALTAN_PATTERN_FLOW_SLOT final
	{
		std::string strSlotId;
		std::string strPatternId;

		bool operator==(const VALTAN_PATTERN_FLOW_SLOT&) const = default;
	};

	struct VALTAN_PATTERN_FLOW_DEFINITION final
	{
		std::string strFlowId;
		std::uint64_t iNextSlotOrdinal = 1u;
		std::uint32_t iInterStepPursuitMs = 1000u;
		std::vector<VALTAN_PATTERN_FLOW_SLOT> Slots;

		bool operator==(const VALTAN_PATTERN_FLOW_DEFINITION&) const = default;
	};

	struct VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 1u;
		std::vector<VALTAN_PATTERN_FLOW_DEFINITION> Flows;

		bool operator==(
			const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT&) const = default;
	};

	/* Saved order shared by Debug playback and Product sequence projection.
	   This document owns durable authoring only; publication and Server
	   revision admission are separate typed commands. */
	class CValtanPatternFlowDocument final
	{
	public:
		// Must match the one-byte shared Pattern Flow wire count.
		static constexpr std::size_t MAX_SLOTS = 255u;
		static constexpr std::uint32_t MIN_INTER_STEP_PURSUIT_MS = 100u;
		static constexpr std::uint32_t MAX_INTER_STEP_PURSUIT_MS = 10000u;
		static constexpr std::string_view DEFAULT_FLOW_ID =
			"flow.valtan.boss-tool.default";

	public:
		static std::filesystem::path Resolve_Path();
		static bool Parse_Text(
			std::string_view text,
			VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& outDocument,
			std::string& outStatus);
		static bool Compute_SourceRevision(
			std::string_view text,
			std::string& outRevision);
		static bool Validate(
			const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& document,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		static std::string Serialize_Text(
			const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& document);

		/* Load/Reload and Save commit baseline + draft only after every disk,
		parse, inventory join, revision, and replacement check succeeds. */
		bool Load(
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Reload(
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Save(
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		/* Read-only CAS preflight used immediately before playback.  It never
		   replaces the admitted draft or baseline. */
		bool Verify_SourceRevision(std::string& outStatus);

		bool Add_Slot(
			std::string_view patternId,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outSlotId,
			std::string& outStatus);
		bool Move_Slot(
			std::string_view slotId,
			std::int32_t delta,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Remove_Slot(
			std::string_view slotId,
			std::string& outStatus);
		bool Set_InterStepPursuitMs(
			std::uint32_t milliseconds,
			std::string& outStatus);

		bool Is_Ready() const noexcept { return m_bReady; }
		bool Is_Dirty() const noexcept;
		bool Has_ExternalConflict() const noexcept
		{
			return m_bExternalConflict;
		}
		const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& Get_Draft() const noexcept
		{
			return m_Draft;
		}
		const VALTAN_PATTERN_FLOW_DEFINITION* Get_DefaultFlow() const noexcept;
		const std::string& Get_SourceRevision() const noexcept
		{
			return m_strSourceRevision;
		}
		const std::filesystem::path& Get_Path() const noexcept
		{
			return m_Path;
		}

	private:
		std::filesystem::path m_Path;
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT m_Baseline;
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT m_Draft;
		std::string m_strSourceRevision;
		bool m_bReady = false;
		bool m_bExternalConflict = false;
	};
}
