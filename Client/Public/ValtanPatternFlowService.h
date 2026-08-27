#pragma once

#include "Client_Defines.h"
#include "GameplayDataRevision.h"
#include "ValtanPatternFlowDocument.h"

#include <cstdint>
#include <string>
#include <string_view>

NS_BEGIN(Client)

enum class VALTAN_PATTERN_FLOW_STATE : uint8_t
{
	IDLE,
	REQUEST_PENDING,
	ACTIVE,
	PAUSED_FOR_REVIVE,
	COMPLETED_HOLD,
	STOPPED_HOLD,
	REJECTED,
	ABORTED,
	END
};

struct VALTAN_PATTERN_FLOW_SNAPSHOT final
{
	VALTAN_PATTERN_FLOW_STATE eState = VALTAN_PATTERN_FLOW_STATE::IDLE;
	uint32_t iRequestSequence = 0u;
	uint32_t iRoomFlowEpoch = 0u;
	uint32_t iPatternSequence = 0u;
	uint16_t iCurrentSlotOrdinal = 0u;
	uint16_t iSlotCount = 0u;
	uint64_t iWorldInboundGeneration = 0u;
	bool_t bStopAfterCurrentRequested = false;
	LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
	std::string strBossPlacementId;
	std::string strFlowId;
	std::string strFlowRevision;
	std::string strStartSlotId;
	std::string strCurrentSlotId;
	std::string strCurrentPatternId;
	std::string strStatus;

	[[nodiscard]] bool_t Is_InFlight() const
	{
		return VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING == eState ||
			VALTAN_PATTERN_FLOW_STATE::ACTIVE == eState ||
			VALTAN_PATTERN_FLOW_STATE::PAUSED_FOR_REVIVE == eState;
	}

	[[nodiscard]] bool_t Is_TerminalHold() const
	{
		return VALTAN_PATTERN_FLOW_STATE::COMPLETED_HOLD == eState ||
			VALTAN_PATTERN_FLOW_STATE::STOPPED_HOLD == eState;
	}
};

/* Owns the one Debug flow result/lifecycle queue.  Boss Tool submits a saved
   document revision and only observes this snapshot; it never sends packets
   or advances individual slots itself.  All methods run on the Client main
   thread. */
class CValtanPatternFlowService final
{
public:
	static CValtanPatternFlowService& Get();

	bool_t Start(
		std::string_view strBossPlacementId,
		const VALTAN_PATTERN_FLOW_DEFINITION& Flow,
		std::string_view strFlowRevision,
		std::string_view strStartSlotId,
		std::string& strOutStatus);
	bool_t Stop_AfterCurrent(std::string& strOutStatus);
	void Update();

	[[nodiscard]] const VALTAN_PATTERN_FLOW_SNAPSHOT& Get_Snapshot() const
	{
		return m_Snapshot;
	}

private:
	CValtanPatternFlowService() = default;
	void Set_Terminal(
		VALTAN_PATTERN_FLOW_STATE eState,
		std::string strStatus);

	VALTAN_PATTERN_FLOW_SNAPSHOT m_Snapshot;
	uint32_t m_iNextRequestSequence = 1u;
	uint32_t m_iNextControlSequence = 1u;
	uint32_t m_iPendingStopControlSequence = 0u;
	uint64_t m_iStateStartedAtMilliseconds = 0u;
	uint64_t m_iPendingStopStartedAtMilliseconds = 0u;
};

const char_t* Describe_ValtanPatternFlowState(
	VALTAN_PATTERN_FLOW_STATE eState);

NS_END
