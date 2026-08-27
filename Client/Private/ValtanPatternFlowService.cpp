#include "ValtanPatternFlowService.h"

#include "NetworkManager.h"
#include "ValtanPatternAuditionService.h"
#include "Network/PacketMessages.h"

#include <Windows.h>
#include <algorithm>
#include <limits>
#include <utility>

namespace
{
	constexpr uint64_t RESULT_TIMEOUT_MILLISECONDS = 5000u;
	constexpr uint64_t START_TIMEOUT_MILLISECONDS = 15000u;

	std::string Describe_Result(
		const LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT Result,
		const std::string& Reason)
	{
		using LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT;
		if (!Reason.empty())
			return Reason;
		switch (Result)
		{
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_RELEASE_BUILD:
			return "Pattern Flow is Debug-only; start the Debug Server.";
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_WRONG_WORLD:
			return "Enter Valtan Arena before starting Pattern Flow.";
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_NO_BOSS:
			return "No replicated Valtan exists at the requested placement.";
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_BOSS_DEAD:
			return "The Server Valtan cannot be reset for Pattern Flow.";
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_PLAYER_NOT_ENGAGED:
			return "A living combat-ready player is required for Pattern Flow.";
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_CONFLICT:
			return "Another Valtan audition is still active in this room.";
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_INVALID_FLOW:
			return "The Server rejected the saved Flow or one of its stable IDs.";
		case VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW:
			return "The Flow command targeted a stale Server flow epoch.";
		case VALTAN_PATTERN_FLOW_RESULT::QUEUED:
		case VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED:
			return "Pattern Flow was accepted by the Server.";
		case VALTAN_PATTERN_FLOW_RESULT::END:
		default:
			return "The Server returned an unknown Pattern Flow verdict.";
		}
	}

	uint32_t Next_Sequence(const uint32_t Current)
	{
		return (std::numeric_limits<uint32_t>::max)() == Current ?
			1u : Current + 1u;
	}
}

Client::CValtanPatternFlowService&
Client::CValtanPatternFlowService::Get()
{
	static CValtanPatternFlowService Instance;
	return Instance;
}

const char_t* Client::Describe_ValtanPatternFlowState(
	const VALTAN_PATTERN_FLOW_STATE State)
{
	switch (State)
	{
	case VALTAN_PATTERN_FLOW_STATE::IDLE: return "IDLE";
	case VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING: return "REQUEST_PENDING";
	case VALTAN_PATTERN_FLOW_STATE::ACTIVE: return "ACTIVE";
	case VALTAN_PATTERN_FLOW_STATE::PAUSED_FOR_REVIVE: return "PAUSED_FOR_REVIVE";
	case VALTAN_PATTERN_FLOW_STATE::COMPLETED_HOLD: return "COMPLETED_HOLD";
	case VALTAN_PATTERN_FLOW_STATE::STOPPED_HOLD: return "STOPPED_HOLD";
	case VALTAN_PATTERN_FLOW_STATE::REJECTED: return "REJECTED";
	case VALTAN_PATTERN_FLOW_STATE::ABORTED: return "ABORTED";
	case VALTAN_PATTERN_FLOW_STATE::END:
	default: return "INVALID";
	}
}

bool_t Client::CValtanPatternFlowService::Start(
	const std::string_view strBossPlacementId,
	const VALTAN_PATTERN_FLOW_DEFINITION& Flow,
	const std::string_view strFlowRevision,
	const std::string_view strStartSlotId,
	std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	Update();
	CValtanPatternAuditionService::Get().Update();
	if (CValtanPatternAuditionService::Get().Has_PlaybackOwnership())
	{
		strOutStatus = "Pattern Flow is locked while an isolated audition or Next command owns playback.";
		return false;
	}
	if (m_Snapshot.Is_InFlight())
	{
		strOutStatus = "Pattern Flow is already " +
			std::string(Describe_ValtanPatternFlowState(m_Snapshot.eState)) + ".";
		return false;
	}
	if (!CNetworkManager::Get().Is_Connected())
	{
		strOutStatus = "Start and connect the Debug Server first.";
		return false;
	}
	if (strBossPlacementId.empty() || Flow.strFlowId.empty() ||
		strFlowRevision.empty() || strStartSlotId.empty() ||
		Flow.Slots.empty() || Flow.Slots.size() > MAX_VALTAN_PATTERN_FLOW_SLOTS)
	{
		strOutStatus = "Pattern Flow requires a saved non-empty bounded Flow and start slot.";
		return false;
	}
	const auto StartSlot = std::find_if(
		Flow.Slots.begin(), Flow.Slots.end(),
		[strStartSlotId](const VALTAN_PATTERN_FLOW_SLOT& Slot)
		{
			return Slot.strSlotId == strStartSlotId;
		});
	if (Flow.Slots.end() == StartSlot)
	{
		strOutStatus = "The selected start slot is not part of the saved Flow.";
		return false;
	}

	C2S_DEBUG_VALTAN_PATTERN_FLOW_START Message{};
	const uint32_t Sequence = 0u == m_iNextRequestSequence ?
		1u : m_iNextRequestSequence;
	Message.iRequestSequence = Sequence;
	Message.strBossPlacementId = std::string(strBossPlacementId);
	Message.strFlowId = Flow.strFlowId;
	Message.strFlowRevision = std::string(strFlowRevision);
	Message.strStartSlotId = std::string(strStartSlotId);
	Message.iInterStepPursuitMs = Flow.iInterStepPursuitMs;
	Message.Slots.reserve(Flow.Slots.size());
	for (const VALTAN_PATTERN_FLOW_SLOT& Slot : Flow.Slots)
	{
		VALTAN_PATTERN_FLOW_SLOT_WIRE Wire{};
		Wire.strSlotId = Slot.strSlotId;
		Wire.strPatternId = Slot.strPatternId;
		Message.Slots.push_back(std::move(Wire));
	}
	if (!CNetworkManager::Get().Send_ValtanPatternFlowStart(Message))
	{
		strOutStatus = "Could not send Pattern Flow to the Server.";
		return false;
	}

	m_iNextRequestSequence = Next_Sequence(Sequence);
	m_iPendingStopControlSequence = 0u;
	m_iPendingStopStartedAtMilliseconds = 0u;
	m_Snapshot = {};
	m_Snapshot.eState = VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING;
	m_Snapshot.iRequestSequence = Sequence;
	m_Snapshot.iWorldInboundGeneration =
		CNetworkManager::Get().Get_WorldInboundGeneration();
	m_Snapshot.iSlotCount = static_cast<uint16_t>(Flow.Slots.size());
	m_Snapshot.strBossPlacementId = std::string(strBossPlacementId);
	m_Snapshot.strFlowId = Flow.strFlowId;
	m_Snapshot.strFlowRevision = std::string(strFlowRevision);
	m_Snapshot.strStartSlotId = std::string(strStartSlotId);
	m_Snapshot.strStatus =
		"Waiting for the Server to validate the saved Flow and perform one arena reset.";
	m_iStateStartedAtMilliseconds = GetTickCount64();
	strOutStatus = m_Snapshot.strStatus;
	return true;
}

bool_t Client::CValtanPatternFlowService::Stop_AfterCurrent(
	std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	Update();
	if (!m_Snapshot.Is_InFlight() || 0u == m_Snapshot.iRoomFlowEpoch ||
		m_Snapshot.bStopAfterCurrentRequested ||
		0u != m_iPendingStopControlSequence)
	{
		strOutStatus = m_Snapshot.bStopAfterCurrentRequested ||
			0u != m_iPendingStopControlSequence ?
			"Stop After Current is already pending." :
			"No active Server Pattern Flow can be stopped.";
		return false;
	}

	C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT Message{};
	const uint32_t Sequence = 0u == m_iNextControlSequence ?
		1u : m_iNextControlSequence;
	Message.iControlSequence = Sequence;
	Message.strFlowId = m_Snapshot.strFlowId;
	Message.iRoomFlowEpoch = m_Snapshot.iRoomFlowEpoch;
	if (!CNetworkManager::Get().Send_ValtanPatternFlowStopAfterCurrent(Message))
	{
		strOutStatus = "Could not send Stop After Current to the Server.";
		return false;
	}
	m_iNextControlSequence = Next_Sequence(Sequence);
	m_iPendingStopControlSequence = Sequence;
	m_iPendingStopStartedAtMilliseconds = GetTickCount64();
	m_Snapshot.bStopAfterCurrentRequested = true;
	m_Snapshot.strStatus =
		"Stop requested. The current Server occurrence will finish normally.";
	strOutStatus = m_Snapshot.strStatus;
	return true;
}

void Client::CValtanPatternFlowService::Set_Terminal(
	const VALTAN_PATTERN_FLOW_STATE State,
	std::string Status)
{
	m_Snapshot.eState = State;
	m_Snapshot.strStatus = std::move(Status);
	m_iPendingStopControlSequence = 0u;
	m_iPendingStopStartedAtMilliseconds = 0u;
	m_iStateStartedAtMilliseconds = GetTickCount64();
}

void Client::CValtanPatternFlowService::Update()
{
	using namespace LostArk::Shared;
	const uint64_t CurrentWorldGeneration =
		CNetworkManager::Get().Get_WorldInboundGeneration();
	if (VALTAN_PATTERN_FLOW_STATE::IDLE != m_Snapshot.eState &&
		m_Snapshot.iWorldInboundGeneration != CurrentWorldGeneration)
	{
		m_Snapshot.iWorldInboundGeneration = CurrentWorldGeneration;
		Set_Terminal(
			VALTAN_PATTERN_FLOW_STATE::ABORTED,
			"Pattern Flow state was cleared because the Client world session changed.");
	}

	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT Result{};
	while (CNetworkManager::Get().Try_Consume_ValtanPatternFlowResult(Result))
	{
		const bool_t IsStart = VALTAN_PATTERN_FLOW_COMMAND::START == Result.eCommand;
		if ((IsStart && VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING !=
				m_Snapshot.eState) ||
			(!IsStart && 0u == m_iPendingStopControlSequence))
		{
			continue;
		}
		const uint32_t ExpectedSequence = IsStart ?
			m_Snapshot.iRequestSequence : m_iPendingStopControlSequence;
		if (0u == ExpectedSequence || Result.iCommandSequence != ExpectedSequence ||
			Result.strFlowId != m_Snapshot.strFlowId ||
			(!Result.strFlowRevision.empty() &&
			 Result.strFlowRevision != m_Snapshot.strFlowRevision))
		{
			if (m_Snapshot.Is_InFlight())
				m_Snapshot.strStatus = "Ignored a mismatched Pattern Flow verdict.";
			continue;
		}
		if (VALTAN_PATTERN_FLOW_RESULT::QUEUED == Result.eResult ||
			VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == Result.eResult)
		{
			m_Snapshot.iRoomFlowEpoch = Result.iRoomFlowEpoch;
			m_Snapshot.PinnedDefinitionRevision = Result.PinnedDefinitionRevision;
			m_Snapshot.strStatus = IsStart ?
				"Server accepted the Flow; waiting for its first slot lifecycle." :
				"Server accepted Stop After Current.";
			if (!IsStart)
			{
				m_iPendingStopControlSequence = 0u;
				m_iPendingStopStartedAtMilliseconds = 0u;
			}
			m_iStateStartedAtMilliseconds = GetTickCount64();
			continue;
		}

		const std::string Status = Describe_Result(Result.eResult, Result.strReason);
		if (IsStart)
			Set_Terminal(VALTAN_PATTERN_FLOW_STATE::REJECTED, Status);
		else
		{
			m_iPendingStopControlSequence = 0u;
			m_iPendingStopStartedAtMilliseconds = 0u;
			m_Snapshot.bStopAfterCurrentRequested = false;
			m_Snapshot.strStatus = "Stop rejected: " + Status;
		}
	}

	S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Lifecycle{};
	while (CNetworkManager::Get().Try_Consume_ValtanPatternFlowLifecycle(Lifecycle))
	{
		if (!m_Snapshot.Is_InFlight())
			continue;
		if (Lifecycle.iRequestSequence != m_Snapshot.iRequestSequence ||
			Lifecycle.strBossPlacementId != m_Snapshot.strBossPlacementId ||
			Lifecycle.strFlowId != m_Snapshot.strFlowId ||
			Lifecycle.strFlowRevision != m_Snapshot.strFlowRevision ||
			Lifecycle.strStartSlotId != m_Snapshot.strStartSlotId)
		{
			if (m_Snapshot.Is_InFlight())
				m_Snapshot.strStatus = "Ignored a lifecycle with different Flow identity.";
			continue;
		}
		if (0u != m_Snapshot.iRoomFlowEpoch &&
			m_Snapshot.iRoomFlowEpoch != Lifecycle.iRoomFlowEpoch)
		{
			Set_Terminal(
				VALTAN_PATTERN_FLOW_STATE::ABORTED,
				"Server Flow epoch changed before this run reached a terminal state.");
			continue;
		}

		m_Snapshot.iRoomFlowEpoch = Lifecycle.iRoomFlowEpoch;
		m_Snapshot.iPatternSequence = Lifecycle.iPatternSequence;
		m_Snapshot.iCurrentSlotOrdinal = Lifecycle.iCurrentSlotOrdinal;
		m_Snapshot.iSlotCount = Lifecycle.iSlotCount;
		m_Snapshot.PinnedDefinitionRevision = Lifecycle.PinnedDefinitionRevision;
		m_Snapshot.strCurrentSlotId = Lifecycle.strCurrentSlotId;
		m_Snapshot.strCurrentPatternId = Lifecycle.strCurrentPatternId;
		const std::string SlotText = 0u == Lifecycle.iCurrentSlotOrdinal ?
			std::string{} :
			" slot " + std::to_string(Lifecycle.iCurrentSlotOrdinal) + "/" +
				std::to_string(Lifecycle.iSlotCount);
		switch (Lifecycle.eState)
		{
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING:
			m_Snapshot.eState = VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING;
			m_Snapshot.strStatus = "Server Flow is pending its first ordered slot.";
			break;
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE:
			m_Snapshot.eState = VALTAN_PATTERN_FLOW_STATE::ACTIVE;
			m_Snapshot.strStatus = "Server Flow ACTIVE" + SlotText + ": " +
				Lifecycle.strCurrentPatternId;
			break;
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PAUSED_FOR_REVIVE:
			m_Snapshot.eState = VALTAN_PATTERN_FLOW_STATE::PAUSED_FOR_REVIVE;
			m_Snapshot.strStatus = "Flow paused for Revive at" + SlotText + ".";
			break;
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD:
			Set_Terminal(
				VALTAN_PATTERN_FLOW_STATE::COMPLETED_HOLD,
				"Flow completed; Valtan is held IDLE until the next audition.");
			break;
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD:
			Set_Terminal(
				VALTAN_PATTERN_FLOW_STATE::STOPPED_HOLD,
				"Flow stopped after the current occurrence; Valtan is held IDLE.");
			break;
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED:
			Set_Terminal(
				VALTAN_PATTERN_FLOW_STATE::REJECTED,
				Lifecycle.strReason.empty() ?
					"Server rejected Pattern Flow." : Lifecycle.strReason);
			break;
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED:
			Set_Terminal(
				VALTAN_PATTERN_FLOW_STATE::ABORTED,
				Lifecycle.strReason.empty() ?
					"Server aborted Pattern Flow." : Lifecycle.strReason);
			break;
		case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::END:
		default:
			continue;
		}
		m_iStateStartedAtMilliseconds = GetTickCount64();
	}

	if (m_Snapshot.Is_InFlight() && !CNetworkManager::Get().Is_Connected())
	{
		Set_Terminal(
			VALTAN_PATTERN_FLOW_STATE::ABORTED,
			"The Server disconnected during Pattern Flow.");
		return;
	}
	const uint64_t Elapsed = GetTickCount64() - m_iStateStartedAtMilliseconds;
	if (VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING == m_Snapshot.eState &&
		Elapsed > START_TIMEOUT_MILLISECONDS)
	{
		Set_Terminal(
			VALTAN_PATTERN_FLOW_STATE::ABORTED,
			"Timed out waiting for the Server Flow lifecycle.");
	}
	if (0u != m_iPendingStopControlSequence &&
		0u != m_iPendingStopStartedAtMilliseconds &&
		GetTickCount64() - m_iPendingStopStartedAtMilliseconds >
			RESULT_TIMEOUT_MILLISECONDS)
	{
		m_iPendingStopControlSequence = 0u;
		m_iPendingStopStartedAtMilliseconds = 0u;
		m_Snapshot.strStatus =
			"Stop verdict timed out; no second control was sent for this Flow.";
	}
}
