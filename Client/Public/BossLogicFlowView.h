#pragma once

#include "ActionCompositionGraphModel.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct VALTAN_PATTERN_VIEW;

	/* Read-only Boss-facing annotation over one immutable Pattern graph node.
	   Gameplay branches remain owned by VALTAN_PATTERN_VIEW; this view only adds
	   the exact animation names and the closed Counter contract needed by the
	   Logic Flow canvas. */
	struct BOSS_LOGIC_FLOW_NODE_VIEW final
	{
		std::size_t iGraphNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		std::string strStageId;
		std::string strActionId;
		std::string strSequenceRole;
		std::string strStageKind;
		std::vector<std::string> ClipNames;
		/* Human-readable, read-only projection of the authored condition inputs.
		   These strings never drive gameplay; they let the large Valtan Logic Pattern
		   inspector show the same proxy/response/status/hit contract the Server
		   consumed from the admitted Pattern. */
		std::vector<std::string> ConditionSummaries;
		bool bCounterWindow = false;
		bool bCounterProxy = false;
		bool bCounterContractIncomplete = false;
	};

	struct BOSS_LOGIC_FLOW_EDGE_VIEW final
	{
		std::size_t iGraphEdgeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		bool bAuthored = false;
		bool bCounterSuccess = false;
		bool bCounterTimeout = false;
	};

	struct BOSS_LOGIC_FLOW_VIEW final
	{
		std::string strPatternId;
		std::string strPatternDisplayName;
		/* Pattern-level authoring fact used by Sequence-line UI badges. This is
		   true for any admitted Stage branch with the exact COUNTER_HIT outcome;
		   it is intentionally independent of the stricter per-node closed Counter
		   contract diagnostics below. */
		bool bHasCounterHitBranch = false;
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Graph;
		std::vector<BOSS_LOGIC_FLOW_NODE_VIEW> Nodes;
		std::vector<BOSS_LOGIC_FLOW_EDGE_VIEW> Edges;
	};

	/* Pure, transactional adapter. A rejected projection preserves the caller's
	   previous view exactly, matching CActionCompositionGraphModel semantics. */
	class CBossLogicFlowViewModel final
	{
	public:
		[[nodiscard]] static bool Has_CounterHitBranch(
			const VALTAN_PATTERN_VIEW& Pattern) noexcept;
		static bool Project(
			const VALTAN_PATTERN_VIEW& Pattern,
			std::uint64_t iSourceGeneration,
			BOSS_LOGIC_FLOW_VIEW& InOutView,
			ACTION_COMPOSITION_GRAPH_ERROR& OutError);
	};

	enum class BOSS_LOGIC_FLOW_SELECTION_KIND : std::uint8_t
	{
		NONE,
		STAGE,
		BRANCH,
	};

	/* Stable selection result returned to the owner. No pointer or vector index
	   escapes the canvas frame. */
	struct BOSS_LOGIC_FLOW_SELECTION final
	{
		BOSS_LOGIC_FLOW_SELECTION_KIND eKind =
			BOSS_LOGIC_FLOW_SELECTION_KIND::NONE;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		std::string strTargetPatternId;
		bool bTerminal = false;
		bool bAuthored = false;

		void Clear();
	};

	/* One immutable Client observation of the already-replicated boss cursor.
	   This is deliberately smaller than WORLD_ENTITY_SNAPSHOT: Valtan Logic Pattern
	   must not grow a second replication/runtime path. */
	struct BOSS_LOGIC_FLOW_LIVE_SNAPSHOT final
	{
		bool bValid = false;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iPatternSequence = 0u;
		std::string strPatternId;
		std::string strActionId;
	};

	/* A branch inferred only from two consecutive authoritative snapshot
	   cursors and one unique immutable graph edge. It is an observation label,
	   never an input to gameplay or Pattern selection. */
	struct BOSS_LOGIC_FLOW_OBSERVED_EDGE final
	{
		std::uint64_t iObservationSequence = 0u;
		std::uint64_t iSourceGeneration = 0u;
		std::uint32_t iSourceServerTick = 0u;
		std::uint32_t iTargetServerTick = 0u;
		std::uint32_t iSourcePatternSequence = 0u;
		std::uint32_t iTargetPatternSequence = 0u;
		std::string strSourcePatternId;
		std::string strSourceStageId;
		std::string strSourceActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		std::string strTargetPatternId;
		bool bTerminal = false;
		bool bCrossPattern = false;
	};

	/* Fail-closed observer over snapshots the Client already admitted. Missing
	   ticks, idle gaps, graph mismatches and ambiguous edges clear the anchor and
	   never manufacture an outcome. History is bounded so an always-open debug
	   window cannot become an unbounded encounter log. */
	class CBossLogicFlowObservedEdgeResolver final
	{
	public:
		static constexpr std::size_t MAX_HISTORY_COUNT = 32u;

		bool Observe(
			const BOSS_LOGIC_FLOW_LIVE_SNAPSHOT& Snapshot,
			const BOSS_LOGIC_FLOW_VIEW* pCurrentView,
			BOSS_LOGIC_FLOW_OBSERVED_EDGE* pOutObserved = nullptr);
		void Reset();
		[[nodiscard]] const std::vector<BOSS_LOGIC_FLOW_OBSERVED_EDGE>&
			Get_History() const noexcept
		{
			return m_History;
		}

	private:
		void Clear_Anchor();

		BOSS_LOGIC_FLOW_LIVE_SNAPSHOT m_PreviousSnapshot;
		BOSS_LOGIC_FLOW_VIEW m_PreviousView;
		std::vector<BOSS_LOGIC_FLOW_OBSERVED_EDGE> m_History;
		std::uint64_t m_iNextObservationSequence = 1u;
		bool m_bHasAnchor = false;
	};

	struct BOSS_LOGIC_FLOW_CANVAS_STATE final
	{
		std::string strPatternId;
		std::uint64_t iSourceGeneration = ~std::uint64_t{ 0u };
		float fPanX = 0.f;
		float fPanY = 0.f;
		float fZoom = 1.f;
		bool bFitRequested = true;
	};

	struct BOSS_LOGIC_FLOW_RENDER_CONTEXT final
	{
		std::string_view strSelectedStageId;
		std::string_view strLiveActionId;
		const BOSS_LOGIC_FLOW_OBSERVED_EDGE* pObservedEdge = nullptr;
		bool bLivePattern = false;
		bool bAllowSelection = false;
		float fMinimumCanvasHeight = 320.f;
	};

	/* Immediate-mode read-only canvas. The Valtan Boss Tool owner supplies live identity;
	   stable click inspection is explicitly opt-in and disabled by default. This
	   renderer never reads the network, mutates gameplay, or owns Pattern selection.
	   Render returns true only when one opt-in stable selection is emitted. */
	class CBossLogicFlowRenderer final
	{
	public:
		static bool Render(
			const BOSS_LOGIC_FLOW_VIEW& View,
			const BOSS_LOGIC_FLOW_RENDER_CONTEXT& Context,
			BOSS_LOGIC_FLOW_CANVAS_STATE& InOutState,
			BOSS_LOGIC_FLOW_SELECTION& OutSelection);
	};
}
