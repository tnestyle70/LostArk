#pragma once

#include "GameplayDataRevision.h"
#include "Network/PacketMessages.h"
#include "ValtanPatternFlowDocument.h"

#include <cstdint>
#include <string>
#include <string_view>

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
#include <deque>
#include <vector>
#endif

namespace Client
{

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
	bool bStopAfterCurrentRequested = false;
	LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
	std::string strBossPlacementId;
	std::string strFlowId;
	std::string strFlowRevision;
	std::string strStartSlotId;
	std::string strCurrentSlotId;
	std::string strCurrentPatternId;
	std::string strStatus;

	[[nodiscard]] bool Is_InFlight() const
	{
		return VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING == eState ||
			VALTAN_PATTERN_FLOW_STATE::ACTIVE == eState ||
			VALTAN_PATTERN_FLOW_STATE::PAUSED_FOR_REVIVE == eState;
	}

	[[nodiscard]] bool Is_TerminalHold() const
	{
		return VALTAN_PATTERN_FLOW_STATE::COMPLETED_HOLD == eState ||
			VALTAN_PATTERN_FLOW_STATE::STOPPED_HOLD == eState;
	}
};

enum class VALTAN_PATTERN_FLOW_START_STATE : uint8_t
{
	NONE,
	WAITING_VERDICT,
	UNCONFIRMED,
	END
};

/* A replacement request never overwrites the admitted run until the Server
   accepts its exact identity. Timeout retains this payload for an exact retry. */
struct VALTAN_PATTERN_FLOW_START_COMMAND final
{
	VALTAN_PATTERN_FLOW_START_STATE eState = VALTAN_PATTERN_FLOW_START_STATE::NONE;
	LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START Request{};
	uint64_t iWorldInboundGeneration = 0u;
	uint64_t iSentAtMilliseconds = 0u;
	std::string strStatus;

	[[nodiscard]] bool Is_Pending() const
	{
		return VALTAN_PATTERN_FLOW_START_STATE::WAITING_VERDICT == eState ||
			VALTAN_PATTERN_FLOW_START_STATE::UNCONFIRMED == eState;
	}
};

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
struct VALTAN_PATTERN_FLOW_HARNESS_INPUT final
{
	bool bConnected = true;
	bool bSendSucceeds = true;
	bool bPendingNextCommand = false;
	uint64_t iWorldInboundGeneration = 1u;
	uint64_t iNowMilliseconds = 100u;
	std::deque<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT> Results;
	std::deque<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE> Lifecycles;
	std::vector<LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START> SentStarts;
	std::vector<LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT> SentStops;
};
#endif

/* Owns the one Debug flow result/lifecycle queue.  Boss Tool submits a saved
   document revision and only observes this snapshot; it never sends packets
   or advances individual slots itself.  All methods run on the Client main
   thread. */
class CValtanPatternFlowService final
{
public:
	static CValtanPatternFlowService& Get();

	bool Start(
		std::string_view strBossPlacementId,
		const VALTAN_PATTERN_FLOW_DEFINITION& Flow,
		std::string_view strFlowRevision,
		std::string_view strStartSlotId,
		std::string& strOutStatus);
	bool Retry_Start(std::string& strOutStatus);
	bool Stop_AfterCurrent(std::string& strOutStatus);
	void Update();

	[[nodiscard]] const VALTAN_PATTERN_FLOW_SNAPSHOT& Get_Snapshot() const
	{
		return m_Snapshot;
	}
	[[nodiscard]] const VALTAN_PATTERN_FLOW_START_COMMAND& Get_PendingStart() const
	{
		return m_PendingStart;
	}
	[[nodiscard]] bool Has_PendingStart() const
	{
		return m_PendingStart.Is_Pending();
	}
	[[nodiscard]] bool Has_PlaybackOwnership() const
	{
		return m_Snapshot.Is_InFlight() || Has_PendingStart() ||
			0u != m_iPendingStopControlSequence;
	}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	void Harness_Reset();
	VALTAN_PATTERN_FLOW_HARNESS_INPUT& Harness_Input() { return m_HarnessInput; }
	void Harness_SetNextRequestSequence(uint32_t iSequence)
	{
		m_iNextRequestSequence = iSequence;
	}
#endif

private:
	CValtanPatternFlowService() = default;
	void Set_Terminal(
		VALTAN_PATTERN_FLOW_STATE eState,
		std::string strStatus);
	void Accept_PendingStart(
		uint32_t iRoomFlowEpoch,
		const LostArk::Shared::GameplayDataRevision& PinnedRevision);
	void Reject_PendingStart(std::string strStatus);
	void Apply_ServerResult(
		const LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& Result);
	void Apply_ServerLifecycle(
		const LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& Lifecycle);
	[[nodiscard]] uint64_t Now_Milliseconds() const;
	[[nodiscard]] bool Is_Connected() const;
	[[nodiscard]] uint64_t World_InboundGeneration() const;
	[[nodiscard]] bool Has_PendingNextCommand() const;
	bool Send_StartRequest(
		const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& Request);
	bool Send_StopRequest(
		const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& Request);
	bool Consume_Result(LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& Result);
	bool Consume_Lifecycle(LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& Lifecycle);

	VALTAN_PATTERN_FLOW_SNAPSHOT m_Snapshot;
	VALTAN_PATTERN_FLOW_START_COMMAND m_PendingStart;
	LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START m_CurrentRequest;
	uint32_t m_iNextRequestSequence = 1u;
	uint32_t m_iNextControlSequence = 1u;
	uint32_t m_iPendingStopControlSequence = 0u;
	uint64_t m_iPendingStopStartedAtMilliseconds = 0u;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	VALTAN_PATTERN_FLOW_HARNESS_INPUT m_HarnessInput;
#endif
};

const char* Describe_ValtanPatternFlowState(
	VALTAN_PATTERN_FLOW_STATE eState);

}
