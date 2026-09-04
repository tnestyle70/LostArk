#pragma once

#include <cstdint>
#include <optional>
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

	enum class VALTAN_PATTERN_FLOW_EDGE_OUTCOME : std::uint8_t
	{
		COMPLETED,
		END
	};

	struct VALTAN_PATTERN_FLOW_NODE final
	{
		std::string strNodeId;
		std::string strPatternId;
		// Zero disables the optional per-pattern watchdog.
		std::uint32_t iWatchdogMs = 0u;

		bool operator==(const VALTAN_PATTERN_FLOW_NODE&) const = default;
	};

	struct VALTAN_PATTERN_FLOW_EDGE final
	{
		std::string strEdgeId;
		std::string strFromNodeId;
		VALTAN_PATTERN_FLOW_EDGE_OUTCOME eOutcome =
			VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED;
		std::string strToNodeId;
		std::uint32_t iPursuitMs = 1000u;
		std::optional<std::uint32_t> iMaxTraversals;

		bool operator==(const VALTAN_PATTERN_FLOW_EDGE&) const = default;
	};

	struct VALTAN_PATTERN_FLOW_DEFINITION final
	{
		std::string strFlowId;
		std::string strEntryNodeId;
		std::uint64_t iNextNodeOrdinal = 1u;
		std::uint64_t iNextEdgeOrdinal = 1u;
		std::uint32_t iDefaultPursuitMs = 1000u;
		std::uint32_t iMaxTransitionsPerRun = 255u;
		std::vector<VALTAN_PATTERN_FLOW_NODE> Nodes;
		std::vector<VALTAN_PATTERN_FLOW_EDGE> Edges;

		/* Transitional read-only projection for the existing Valtan Boss Tool and
		   ordered Server audition command. It is populated only when the v2
		   graph is one acyclic COMPLETED chain. Per-transition pursuit remains
		   owned by Edges and is projected into the canonical scriptedSequence.
		   This adapter is never a second serialized owner. */
		std::uint64_t iNextSlotOrdinal = 1u;
		std::uint32_t iInterStepPursuitMs = 1000u;
		std::vector<VALTAN_PATTERN_FLOW_SLOT> Slots;

		bool operator==(const VALTAN_PATTERN_FLOW_DEFINITION&) const = default;
	};

	struct VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT final
	{
		std::uint32_t iFormatVersion = 2u;
		std::vector<VALTAN_PATTERN_FLOW_DEFINITION> Flows;

		bool operator==(
			const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT&) const = default;
	};

	/* In-memory Valtan Boss Tool projection of the canonical gameplay
	   scriptedSequence.  Durable authoring is owned exclusively by the shared
	   Valtan gameplay transaction; this adapter never reads or writes a second
	   Flow file. */
	class CValtanPatternFlowDocument final
	{
	public:
		// Must match the one-byte shared Pattern Flow wire count.
		static constexpr std::size_t MAX_NODES = 255u;
		static constexpr std::size_t MAX_EDGES = 255u;
		static constexpr std::size_t MAX_SLOTS = MAX_NODES;
		static constexpr std::uint32_t MIN_INTER_STEP_PURSUIT_MS = 100u;
		static constexpr std::uint32_t MAX_INTER_STEP_PURSUIT_MS = 10000u;
		static constexpr std::uint32_t MIN_NODE_WATCHDOG_MS = 1000u;
		static constexpr std::uint32_t MAX_NODE_WATCHDOG_MS = 300000u;
		static constexpr std::uint32_t DEFAULT_NODE_WATCHDOG_MS = 0u;
		static constexpr std::uint32_t MIN_TRANSITIONS_PER_RUN = 1u;
		static constexpr std::uint32_t MAX_TRANSITIONS_PER_RUN = 4096u;
		static constexpr std::uint32_t MAX_EDGE_TRAVERSALS = 255u;
		static constexpr std::string_view DEFAULT_FLOW_ID =
			"flow.valtan.boss-tool.default";

	public:
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

		/* Stage the canonical gameplay scriptedSequence as the existing linear
		   Valtan Boss Tool view.  Node/edge IDs are deterministic editor projections and
		   are never another durable source document. */
		bool Load_CanonicalSequence(
			std::string_view sequenceId,
			std::string_view mode,
			std::uint32_t interStepPursuitMs,
			const std::vector<std::uint32_t>& transitionPursuitMs,
			const std::vector<std::string>& patternIds,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		/* Read-only preflight for the staged canonical sequence. The complete
		   Product/source revision is separately admitted by Valtan Boss Tool before the
		   Server command. */
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
		bool Set_NodeWatchdogMs(
			std::string_view nodeId,
			std::uint32_t milliseconds,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Set_MaxTransitionsPerRun(
			std::uint32_t transitions,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		/* Graph authoring mutations are staged on a copy and replace the live
		   draft only after the complete v2 graph validates.  A rejected edit
		   therefore never leaves an unreachable half-wire in memory. */
		bool Insert_Node_After(
			std::string_view afterNodeId,
			std::string_view patternId,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outNodeId,
			std::string& outStatus);
		bool Remove_Node(
			std::string_view nodeId,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Set_EntryNode(
			std::string_view nodeId,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Connect_CompletedEdge(
			std::string_view fromNodeId,
			std::string_view toNodeId,
			std::uint32_t pursuitMs,
			std::uint32_t maximumTraversals,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outEdgeId,
			std::string& outStatus);
		bool Remove_Edge(
			std::string_view edgeId,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Set_EdgePursuitMs(
			std::string_view edgeId,
			std::uint32_t pursuitMs,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);
		bool Set_EdgeMaxTraversals(
			std::string_view edgeId,
			std::uint32_t maximumTraversals,
			const std::vector<std::string>& admittedPatternIds,
			std::string& outStatus);

		static bool Has_LegacyLinearProjection(
			const VALTAN_PATTERN_FLOW_DEFINITION& flow) noexcept;

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
		const VALTAN_PATTERN_FLOW_DEFINITION* Get_SavedDefaultFlow() const noexcept;
		const std::string& Get_SourceRevision() const noexcept
		{
			return m_strSourceRevision;
		}
	private:
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT m_Baseline;
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT m_Draft;
		std::string m_strSourceRevision;
		bool m_bReady = false;
		bool m_bExternalConflict = false;
	};
}
