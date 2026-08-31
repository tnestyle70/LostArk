#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace Client
{
	struct VALTAN_PATTERN_VIEW;

	inline constexpr std::size_t ACTION_COMPOSITION_GRAPH_INVALID_INDEX =
		(std::numeric_limits<std::size_t>::max)();

	struct ACTION_COMPOSITION_GRAPH_POINT final
	{
		float fX = 0.f;
		float fY = 0.f;
	};

	struct ACTION_COMPOSITION_GRAPH_RECT final
	{
		float fLeft = 0.f;
		float fTop = 0.f;
		float fRight = 0.f;
		float fBottom = 0.f;

		bool Contains(const ACTION_COMPOSITION_GRAPH_POINT& Point) const noexcept;
	};

	struct ACTION_COMPOSITION_GRAPH_NODE_KEY final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;

		bool operator==(const ACTION_COMPOSITION_GRAPH_NODE_KEY& Right) const noexcept;
	};

	struct ACTION_COMPOSITION_GRAPH_NODE final
	{
		ACTION_COMPOSITION_GRAPH_NODE_KEY Key;
		std::string strSequenceRole;
		std::string strStageKind;
		std::uint32_t iDurationMs = 0u;
		std::size_t iCanonicalStageIndex = 0u;
		std::size_t iGraphDepth = 0u;
		std::size_t iGameplayActionCount = 0u;
		std::size_t iBranchCount = 0u;
		std::size_t iAnimationOccurrenceCount = 0u;
		std::size_t iEffectReferenceCount = 0u;
		std::size_t iCameraInvocationCount = 0u;
		bool bEventEntered = false;
		bool bReachable = false;
		ACTION_COMPOSITION_GRAPH_RECT Bounds;
	};

	enum class ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN : std::uint8_t
	{
		AUTHORED_BRANCH,
		DERIVED_TIMEOUT,
	};

	struct ACTION_COMPOSITION_GRAPH_EDGE final
	{
		std::string strPatternId;
		std::string strSourceActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		std::size_t iSourceNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		std::size_t iTargetNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		/* Branch order is part of the canonical Server projection.  Authored
		   edges keep their source Stage branch ordinal so UI hit handling can
		   revalidate the full stable source identity before selecting it. */
		std::size_t iSourceBranchIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN eOrigin =
			ACTION_COMPOSITION_GRAPH_EDGE_ORIGIN::AUTHORED_BRANCH;
		bool bTerminal = false;
		bool bDefault = false;
		std::vector<ACTION_COMPOSITION_GRAPH_POINT> Polyline;
	};

	struct ACTION_COMPOSITION_GRAPH_PATH final
	{
		std::vector<std::size_t> NodeIndices;
		std::vector<std::size_t> EdgeIndices;
		std::uint64_t iDurationMs = 0u;
		bool bTerminal = false;
	};

	/* One preview-only branch choice for a Stage action. It never mutates the
	   admitted Pattern and is applied only while deriving SelectedPath. */
	struct ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE final
	{
		std::string strActionId;
		std::string strOutcome;
	};

	struct ACTION_COMPOSITION_GRAPH_SNAPSHOT final
	{
		std::uint64_t iSourceGeneration = 0u;
		std::string strPatternId;
		std::size_t iEntryNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		std::vector<ACTION_COMPOSITION_GRAPH_NODE> Nodes;
		std::vector<ACTION_COMPOSITION_GRAPH_EDGE> Edges;
		ACTION_COMPOSITION_GRAPH_PATH DefaultPath;
		ACTION_COMPOSITION_GRAPH_PATH SelectedPath;
		ACTION_COMPOSITION_GRAPH_PATH MaximumPath;
		ACTION_COMPOSITION_GRAPH_RECT GraphBounds;
	};

	enum class ACTION_COMPOSITION_GRAPH_ERROR_CODE : std::uint8_t
	{
		NONE,
		INVALID_PATTERN_ID,
		INVALID_PATTERN_ACTION_ID,
		EMPTY_STAGE_SET,
		INVALID_STAGE_ID,
		DUPLICATE_STAGE_ID,
		INVALID_ACTION_ID,
		DUPLICATE_ACTION_ID,
		INVALID_ENTRY_ACTION_ID,
		ENTRY_IS_NOT_FIRST_ACTION,
		INVALID_OUTCOME,
		DUPLICATE_OUTCOME,
		DANGLING_TARGET_ACTION,
		DUPLICATE_OUTCOME_OVERRIDE,
		UNKNOWN_OVERRIDE_ACTION,
		UNKNOWN_OVERRIDE_OUTCOME,
		CYCLE,
		DURATION_OVERFLOW,
	};

	struct ACTION_COMPOSITION_GRAPH_ERROR final
	{
		ACTION_COMPOSITION_GRAPH_ERROR_CODE eCode =
			ACTION_COMPOSITION_GRAPH_ERROR_CODE::NONE;
		std::string strMessage;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::string strOutcome;

		void Clear();
	};

	/* Pure projection over one immutable, already-admitted gameplay Pattern.
	   The output snapshot is committed only after validation, topology, paths,
	   layout, and hit-test geometry all succeed. */
	class CActionCompositionGraphModel final
	{
	public:
		static bool Project(
			const VALTAN_PATTERN_VIEW& Pattern,
			std::uint64_t iSourceGeneration,
			const std::vector<ACTION_COMPOSITION_GRAPH_OUTCOME_OVERRIDE>&
				OutcomeOverrides,
			ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
			ACTION_COMPOSITION_GRAPH_ERROR& OutError);

		static bool Project(
			const VALTAN_PATTERN_VIEW& Pattern,
			std::uint64_t iSourceGeneration,
			ACTION_COMPOSITION_GRAPH_SNAPSHOT& InOutSnapshot,
			ACTION_COMPOSITION_GRAPH_ERROR& OutError);

		static std::size_t Hit_TestNode(
			const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
			const ACTION_COMPOSITION_GRAPH_POINT& GraphPoint) noexcept;

		static std::size_t Hit_TestEdge(
			const ACTION_COMPOSITION_GRAPH_SNAPSHOT& Snapshot,
			const ACTION_COMPOSITION_GRAPH_POINT& GraphPoint,
			float fTolerance = 6.f) noexcept;
	};
}
