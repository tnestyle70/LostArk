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
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Graph;
		std::vector<BOSS_LOGIC_FLOW_NODE_VIEW> Nodes;
		std::vector<BOSS_LOGIC_FLOW_EDGE_VIEW> Edges;
	};

	/* Pure, transactional adapter. A rejected projection preserves the caller's
	   previous view exactly, matching CActionCompositionGraphModel semantics. */
	class CBossLogicFlowViewModel final
	{
	public:
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
		bool bTerminal = false;
		bool bAuthored = false;

		void Clear();
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
		bool bLivePattern = false;
		bool bAllowSelection = false;
		float fMinimumCanvasHeight = 320.f;
	};

	/* Immediate-mode read-only canvas. The Boss Tool owner supplies live identity;
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
