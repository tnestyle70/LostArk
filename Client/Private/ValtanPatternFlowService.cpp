#include "ValtanPatternFlowService.h"

#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
#include "NetworkManager.h"
#include "ValtanPatternAuditionService.h"
#endif
#include "Network/PacketWriter.h"

#include <Windows.h>
#include <algorithm>
#include <limits>
#include <unordered_set>
#include <utility>

namespace
{
	static_assert(
		Client::CValtanPatternFlowDocument::MAX_SLOTS ==
			LostArk::Shared::MAX_VALTAN_PATTERN_FLOW_SLOTS,
		"Saved Flow and wire slot limits must remain identical.");

	constexpr uint64_t RESULT_TIMEOUT_MILLISECONDS = 5000u;

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
			return "Another Valtan audition owns playback in this room.";
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
			0u : Current + 1u;
	}

	bool Matches_Request(
		const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& Request,
		const LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& Lifecycle)
	{
		return Request.iRequestSequence == Lifecycle.iRequestSequence &&
			Request.strBossPlacementId == Lifecycle.strBossPlacementId &&
			Request.strFlowId == Lifecycle.strFlowId &&
			Request.strFlowRevision == Lifecycle.strFlowRevision &&
			Request.strStartSlotId == Lifecycle.strStartSlotId;
	}

	bool Matches_OrderedSlot(
		const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& Request,
		const LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& Lifecycle)
	{
		if (Lifecycle.iSlotCount != Request.Slots.size() ||
			0u == Lifecycle.iCurrentSlotOrdinal ||
			Lifecycle.iCurrentSlotOrdinal > Request.Slots.size())
		{
			return false;
		}
		const auto Start = std::find_if(Request.Slots.begin(), Request.Slots.end(),
			[&Request](const LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE& Slot)
			{
				return Slot.strSlotId == Request.strStartSlotId;
			});
		const std::size_t Index = Lifecycle.iCurrentSlotOrdinal - 1u;
		return Start != Request.Slots.end() &&
			Index >= static_cast<std::size_t>(Start - Request.Slots.begin()) &&
			Request.Slots[Index].strSlotId == Lifecycle.strCurrentSlotId &&
			Request.Slots[Index].strPatternId == Lifecycle.strCurrentPatternId;
	}

	bool Is_TerminalLifecycle(
		const LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE State)
	{
		using namespace LostArk::Shared;
		return VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD == State ||
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD == State ||
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED == State ||
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED == State;
	}

	bool Can_IntroduceOccurrence(
		const LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE State)
	{
		using namespace LostArk::Shared;
		return VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING == State ||
			VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE == State;
	}

	bool Is_ForwardOccurrenceSequence(
		const uint32_t Candidate,
		const uint32_t Previous)
	{
		if (0u == Candidate)
			return false;
		if (0u == Previous)
			return true;
		return static_cast<int32_t>(Candidate - Previous) > 0;
	}

	bool Accepts_LifecycleCursor(
		const Client::VALTAN_PATTERN_FLOW_SNAPSHOT& Snapshot,
		const LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& Lifecycle)
	{
		using namespace LostArk::Shared;
		if (0u == Lifecycle.iPatternSequence)
			return false;
		if (0u == Snapshot.iPatternSequence || 0u == Snapshot.iCurrentSlotOrdinal)
			return true;

		const bool SameSlot =
			Snapshot.iCurrentSlotOrdinal == Lifecycle.iCurrentSlotOrdinal &&
			Snapshot.strCurrentSlotId == Lifecycle.strCurrentSlotId &&
			Snapshot.strCurrentPatternId == Lifecycle.strCurrentPatternId;
		const bool SameSequence =
			Snapshot.iPatternSequence == Lifecycle.iPatternSequence;
		if (SameSlot && SameSequence)
		{
			/* A duplicate PENDING cannot roll an already observed occurrence back.
			   ACTIVE after PAUSED is a valid resume of the same occurrence. */
			return VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING != Lifecycle.eState ||
				(Client::VALTAN_PATTERN_FLOW_STATE::ACTIVE != Snapshot.eState &&
				 Client::VALTAN_PATTERN_FLOW_STATE::PAUSED_FOR_REVIVE != Snapshot.eState);
		}

		/* Terminal messages reuse the Server's last emitted slot/sequence cursor.
		   They must never invent a later occurrence or terminate an older one. */
		if (Is_TerminalLifecycle(Lifecycle.eState) ||
			!Can_IntroduceOccurrence(Lifecycle.eState) ||
			!Is_ForwardOccurrenceSequence(
				Lifecycle.iPatternSequence, Snapshot.iPatternSequence))
		{
			return false;
		}
		if (Lifecycle.iCurrentSlotOrdinal < Snapshot.iCurrentSlotOrdinal)
			return false;
		return Lifecycle.iCurrentSlotOrdinal != Snapshot.iCurrentSlotOrdinal || SameSlot;
	}
}

Client::CValtanPatternFlowService&
Client::CValtanPatternFlowService::Get()
{
	static CValtanPatternFlowService Instance;
	return Instance;
}

const char* Client::Describe_ValtanPatternFlowState(
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

bool Client::CValtanPatternFlowService::Start(
	const std::string_view strBossPlacementId,
	const VALTAN_PATTERN_FLOW_DEFINITION& Flow,
	const std::string_view strFlowRevision,
	const std::string_view strStartSlotId,
	std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	Update();
#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	CValtanPatternAuditionService::Get().Update();
#endif
	if (Has_PendingStart())
	{
		strOutStatus = "A Flow start is still pending. Reconfirm that same request before another restart.";
		return false;
	}
	if (Has_PendingNextCommand())
	{
		strOutStatus = "Wait for the pending Next command verdict before restarting Flow.";
		return false;
	}
	if (!Is_Connected())
	{
		strOutStatus = "Start and connect the Debug Server first.";
		return false;
	}
	if (0u == m_iNextRequestSequence)
	{
		strOutStatus = "Flow request identities are exhausted; restart the Client session.";
		return false;
	}
	if (Flow.Slots.empty() || Flow.Slots.size() > MAX_VALTAN_PATTERN_FLOW_SLOTS)
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

	VALTAN_PATTERN_FLOW_START_COMMAND Staged;
	Staged.eState = VALTAN_PATTERN_FLOW_START_STATE::WAITING_VERDICT;
	Staged.iWorldInboundGeneration = World_InboundGeneration();
	Staged.iSentAtMilliseconds = Now_Milliseconds();
	Staged.strStatus = "Waiting for Server acceptance. The current run remains authoritative until the saved Flow replaces it.";
	C2S_DEBUG_VALTAN_PATTERN_FLOW_START& Message = Staged.Request;
	Message.iRequestSequence = m_iNextRequestSequence;
	Message.strBossPlacementId = strBossPlacementId;
	Message.strFlowId = Flow.strFlowId;
	Message.strFlowRevision = strFlowRevision;
	Message.strStartSlotId = strStartSlotId;
	Message.iInterStepPursuitMs = Flow.iInterStepPursuitMs;
	Message.Slots.reserve(Flow.Slots.size());
	std::unordered_set<std::string> SlotIds;
	for (const VALTAN_PATTERN_FLOW_SLOT& Slot : Flow.Slots)
	{
		if (!SlotIds.insert(Slot.strSlotId).second)
		{
			strOutStatus = "Pattern Flow contains a duplicate stable slot ID.";
			return false;
		}
		Message.Slots.push_back({ Slot.strSlotId, Slot.strPatternId });
	}
	CPacketWriter ValidationWriter;
	if (!Write_Message(ValidationWriter, Message))
	{
		strOutStatus = "Pattern Flow has an invalid revision, stable ID, or pursuit interval.";
		return false;
	}
	if (!Send_StartRequest(Message))
	{
		strOutStatus = "Could not send Pattern Flow to the Server; the current run was preserved.";
		return false;
	}

	m_iNextRequestSequence = Next_Sequence(m_iNextRequestSequence);
	m_PendingStart = std::move(Staged);
	strOutStatus = m_PendingStart.strStatus;
	return true;
}

bool Client::CValtanPatternFlowService::Retry_Start(std::string& strOutStatus)
{
	Update();
	if (!Has_PendingStart() ||
		VALTAN_PATTERN_FLOW_START_STATE::UNCONFIRMED != m_PendingStart.eState)
	{
		strOutStatus = "No unconfirmed Flow start is waiting for an exact retry.";
		return false;
	}
	if (!Send_StartRequest(m_PendingStart.Request))
	{
		strOutStatus = "Could not reconfirm the same Flow request; its playback ownership is preserved.";
		return false;
	}
	m_PendingStart.eState = VALTAN_PATTERN_FLOW_START_STATE::WAITING_VERDICT;
	m_PendingStart.iSentAtMilliseconds = Now_Milliseconds();
	m_PendingStart.strStatus = "Reconfirming the same Flow request without issuing another restart.";
	strOutStatus = m_PendingStart.strStatus;
	return true;
}

bool Client::CValtanPatternFlowService::Stop_AfterCurrent(std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	Update();
	if (Has_PendingStart())
	{
		strOutStatus = "Wait for the Flow restart verdict before sending Stop After Current.";
		return false;
	}
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
	if (0u == m_iNextControlSequence)
	{
		strOutStatus = "Flow control identities are exhausted; restart the Client session.";
		return false;
	}

	C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT Message{};
	Message.iControlSequence = m_iNextControlSequence;
	Message.strFlowId = m_Snapshot.strFlowId;
	Message.iRoomFlowEpoch = m_Snapshot.iRoomFlowEpoch;
	if (!Send_StopRequest(Message))
	{
		strOutStatus = "Could not send Stop After Current to the Server.";
		return false;
	}
	m_iNextControlSequence = Next_Sequence(m_iNextControlSequence);
	m_iPendingStopControlSequence = Message.iControlSequence;
	m_iPendingStopStartedAtMilliseconds = Now_Milliseconds();
	m_Snapshot.bStopAfterCurrentRequested = true;
	m_Snapshot.strStatus = "Stop requested. The current Server occurrence will finish normally.";
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
}

void Client::CValtanPatternFlowService::Accept_PendingStart(
	const uint32_t iRoomFlowEpoch,
	const LostArk::Shared::GameplayDataRevision& PinnedRevision)
{
	VALTAN_PATTERN_FLOW_SNAPSHOT Staged;
	const auto& Request = m_PendingStart.Request;
	Staged.eState = VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING;
	Staged.iRequestSequence = Request.iRequestSequence;
	Staged.iRoomFlowEpoch = iRoomFlowEpoch;
	Staged.iWorldInboundGeneration = m_PendingStart.iWorldInboundGeneration;
	Staged.iSlotCount = static_cast<uint16_t>(Request.Slots.size());
	Staged.strBossPlacementId = Request.strBossPlacementId;
	Staged.strFlowId = Request.strFlowId;
	Staged.strFlowRevision = Request.strFlowRevision;
	Staged.strStartSlotId = Request.strStartSlotId;
	Staged.PinnedDefinitionRevision = PinnedRevision;
	Staged.strStatus = "Server accepted the saved Flow; waiting for its first slot lifecycle.";
	m_CurrentRequest = std::move(m_PendingStart.Request);
	m_Snapshot = std::move(Staged);
	m_PendingStart = {};
	m_iPendingStopControlSequence = 0u;
	m_iPendingStopStartedAtMilliseconds = 0u;
}

void Client::CValtanPatternFlowService::Reject_PendingStart(std::string strStatus)
{
	/* The old run may have advanced or finished while this request was pending.
	   Keep that latest admitted snapshot and its outstanding stop control. */
	m_PendingStart = {};
	m_PendingStart.strStatus = "Flow start rejected: " + std::move(strStatus);
}

void Client::CValtanPatternFlowService::Apply_ServerResult(
	const LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& Result)
{
	using namespace LostArk::Shared;
	if (static_cast<uint8_t>(Result.eResult) >=
		static_cast<uint8_t>(VALTAN_PATTERN_FLOW_RESULT::END))
	{
		return;
	}
	const bool Accepted = VALTAN_PATTERN_FLOW_RESULT::QUEUED == Result.eResult ||
		VALTAN_PATTERN_FLOW_RESULT::DUPLICATE_IGNORED == Result.eResult;
	if (VALTAN_PATTERN_FLOW_COMMAND::START == Result.eCommand)
	{
		if (!Has_PendingStart() ||
			Result.iCommandSequence != m_PendingStart.Request.iRequestSequence ||
			Result.strFlowId != m_PendingStart.Request.strFlowId ||
			Result.strFlowRevision != m_PendingStart.Request.strFlowRevision)
		{
			return;
		}
		if (Accepted)
		{
			if (0u != Result.iRoomFlowEpoch && Result.PinnedDefinitionRevision.Is_Valid())
				Accept_PendingStart(Result.iRoomFlowEpoch, Result.PinnedDefinitionRevision);
		}
		else
		{
			Reject_PendingStart(Describe_Result(Result.eResult, Result.strReason));
		}
		return;
	}
	if (VALTAN_PATTERN_FLOW_COMMAND::STOP_AFTER_CURRENT != Result.eCommand ||
		0u == m_iPendingStopControlSequence ||
		Result.iCommandSequence != m_iPendingStopControlSequence ||
		Result.strFlowId != m_Snapshot.strFlowId ||
		(!Result.strFlowRevision.empty() && Result.strFlowRevision != m_Snapshot.strFlowRevision))
	{
		return;
	}
	if (Accepted && (Result.iRoomFlowEpoch != m_Snapshot.iRoomFlowEpoch ||
		Result.PinnedDefinitionRevision != m_Snapshot.PinnedDefinitionRevision))
	{
		return;
	}
	m_iPendingStopControlSequence = 0u;
	m_iPendingStopStartedAtMilliseconds = 0u;
	if (Accepted)
		m_Snapshot.strStatus = "Server accepted Stop After Current.";
	else
	{
		m_Snapshot.bStopAfterCurrentRequested = false;
		m_Snapshot.strStatus = "Stop rejected: " + Describe_Result(Result.eResult, Result.strReason);
	}
}

void Client::CValtanPatternFlowService::Apply_ServerLifecycle(
	const LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& Lifecycle)
{
	using namespace LostArk::Shared;
	if (static_cast<uint8_t>(Lifecycle.eState) >=
		static_cast<uint8_t>(VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::END))
	{
		return;
	}
	if (Has_PendingStart() && Matches_Request(m_PendingStart.Request, Lifecycle))
	{
		if (VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED == Lifecycle.eState)
		{
			Reject_PendingStart(Lifecycle.strReason);
			return;
		}
		if (0u == Lifecycle.iRoomFlowEpoch || !Lifecycle.PinnedDefinitionRevision.Is_Valid() ||
			!Matches_OrderedSlot(m_PendingStart.Request, Lifecycle))
		{
			return;
		}
		Accept_PendingStart(Lifecycle.iRoomFlowEpoch, Lifecycle.PinnedDefinitionRevision);
	}
	if (!m_Snapshot.Is_InFlight())
		return;
	if (!Matches_Request(m_CurrentRequest, Lifecycle) ||
		m_Snapshot.iRoomFlowEpoch != Lifecycle.iRoomFlowEpoch ||
		m_Snapshot.PinnedDefinitionRevision != Lifecycle.PinnedDefinitionRevision ||
		!Matches_OrderedSlot(m_CurrentRequest, Lifecycle))
	{
		return;
	}

	if (!Accepts_LifecycleCursor(m_Snapshot, Lifecycle))
		return;

	m_Snapshot.iPatternSequence = Lifecycle.iPatternSequence;
	m_Snapshot.iCurrentSlotOrdinal = Lifecycle.iCurrentSlotOrdinal;
	m_Snapshot.iSlotCount = Lifecycle.iSlotCount;
	m_Snapshot.strCurrentSlotId = Lifecycle.strCurrentSlotId;
	m_Snapshot.strCurrentPatternId = Lifecycle.strCurrentPatternId;
	const std::string SlotText = " slot " + std::to_string(Lifecycle.iCurrentSlotOrdinal) +
		"/" + std::to_string(Lifecycle.iSlotCount);
	switch (Lifecycle.eState)
	{
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PENDING:
		m_Snapshot.eState = VALTAN_PATTERN_FLOW_STATE::REQUEST_PENDING;
		m_Snapshot.strStatus = "Server Flow is waiting at" + SlotText + ".";
		break;
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ACTIVE:
		m_Snapshot.eState = VALTAN_PATTERN_FLOW_STATE::ACTIVE;
		m_Snapshot.strStatus = "Server Flow ACTIVE" + SlotText + ": " + Lifecycle.strCurrentPatternId;
		break;
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::PAUSED_FOR_REVIVE:
		m_Snapshot.eState = VALTAN_PATTERN_FLOW_STATE::PAUSED_FOR_REVIVE;
		m_Snapshot.strStatus = "Flow paused for Revive at" + SlotText + ".";
		break;
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::COMPLETED_HOLD:
		Set_Terminal(VALTAN_PATTERN_FLOW_STATE::COMPLETED_HOLD,
			"Flow completed; Valtan is held IDLE until the next audition.");
		break;
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::STOPPED_HOLD:
		Set_Terminal(VALTAN_PATTERN_FLOW_STATE::STOPPED_HOLD,
			"Flow stopped after the current occurrence; Valtan is held IDLE.");
		break;
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::REJECTED:
		Set_Terminal(VALTAN_PATTERN_FLOW_STATE::REJECTED,
			Lifecycle.strReason.empty() ? "Server rejected Pattern Flow." : Lifecycle.strReason);
		break;
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED:
		Set_Terminal(VALTAN_PATTERN_FLOW_STATE::ABORTED,
			Lifecycle.strReason.empty() ? "Server aborted Pattern Flow." : Lifecycle.strReason);
		break;
	case VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::END:
	default:
		break;
	}
}

void Client::CValtanPatternFlowService::Update()
{
	using namespace LostArk::Shared;
	const uint64_t CurrentWorldGeneration = World_InboundGeneration();
	const bool ChangedWorld =
		(VALTAN_PATTERN_FLOW_STATE::IDLE != m_Snapshot.eState &&
		 m_Snapshot.iWorldInboundGeneration != CurrentWorldGeneration) ||
		(Has_PendingStart() && m_PendingStart.iWorldInboundGeneration != CurrentWorldGeneration);
	const bool Disconnected = !Is_Connected();
	if (ChangedWorld || Disconnected)
	{
		const std::string Reason = ChangedWorld ?
			"Pattern Flow state was cleared because the Client world session changed." :
			"The Server disconnected during Pattern Flow.";
		if (VALTAN_PATTERN_FLOW_STATE::IDLE != m_Snapshot.eState)
		{
			m_Snapshot.iWorldInboundGeneration = CurrentWorldGeneration;
			if (ChangedWorld || m_Snapshot.Is_InFlight())
				Set_Terminal(VALTAN_PATTERN_FLOW_STATE::ABORTED, Reason);
		}
		if (Has_PendingStart())
			Reject_PendingStart(Reason);
		m_CurrentRequest = {};
		m_iPendingStopControlSequence = 0u;
		m_iPendingStopStartedAtMilliseconds = 0u;
	}

	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT Result{};
	while (Consume_Result(Result))
	{
		if (!ChangedWorld && !Disconnected)
			Apply_ServerResult(Result);
	}
	S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Lifecycle{};
	while (Consume_Lifecycle(Lifecycle))
	{
		if (!ChangedWorld && !Disconnected)
			Apply_ServerLifecycle(Lifecycle);
	}

	if (Has_PendingStart() &&
		VALTAN_PATTERN_FLOW_START_STATE::WAITING_VERDICT == m_PendingStart.eState &&
		Now_Milliseconds() - m_PendingStart.iSentAtMilliseconds > RESULT_TIMEOUT_MILLISECONDS)
	{
		m_PendingStart.eState = VALTAN_PATTERN_FLOW_START_STATE::UNCONFIRMED;
		m_PendingStart.strStatus = "Flow start acceptance is unconfirmed. Reconfirm the same request; the current run and request ownership are preserved.";
	}
	/* A Server-admitted PENDING may wait between slots for manual revive.
	   A local timer cannot cancel that Server program or release ownership. */
	if (0u != m_iPendingStopControlSequence &&
		0u != m_iPendingStopStartedAtMilliseconds &&
		Now_Milliseconds() - m_iPendingStopStartedAtMilliseconds > RESULT_TIMEOUT_MILLISECONDS)
	{
		m_iPendingStopStartedAtMilliseconds = 0u;
		m_Snapshot.strStatus = "Stop verdict is unconfirmed; no second control was sent for this Flow. Waiting for its Server lifecycle.";
	}
}

uint64_t Client::CValtanPatternFlowService::Now_Milliseconds() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.iNowMilliseconds;
#else
	return GetTickCount64();
#endif
}

bool Client::CValtanPatternFlowService::Is_Connected() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.bConnected;
#else
	return CNetworkManager::Get().Is_Connected();
#endif
}

uint64_t Client::CValtanPatternFlowService::World_InboundGeneration() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.iWorldInboundGeneration;
#else
	return CNetworkManager::Get().Get_WorldInboundGeneration();
#endif
}

bool Client::CValtanPatternFlowService::Has_PendingNextCommand() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.bPendingNextCommand;
#else
	return CValtanPatternAuditionService::Get().Has_PendingNextCommand();
#endif
}

bool Client::CValtanPatternFlowService::Send_StartRequest(
	const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& Request)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (!m_HarnessInput.bSendSucceeds)
		return false;
	m_HarnessInput.SentStarts.push_back(Request);
	return true;
#else
	return CNetworkManager::Get().Send_ValtanPatternFlowStart(Request);
#endif
}

bool Client::CValtanPatternFlowService::Send_StopRequest(
	const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& Request)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (!m_HarnessInput.bSendSucceeds)
		return false;
	m_HarnessInput.SentStops.push_back(Request);
	return true;
#else
	return CNetworkManager::Get().Send_ValtanPatternFlowStopAfterCurrent(Request);
#endif
}

bool Client::CValtanPatternFlowService::Consume_Result(
	LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT& Result)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (m_HarnessInput.Results.empty())
		return false;
	Result = std::move(m_HarnessInput.Results.front());
	m_HarnessInput.Results.pop_front();
	return true;
#else
	return CNetworkManager::Get().Try_Consume_ValtanPatternFlowResult(Result);
#endif
}

bool Client::CValtanPatternFlowService::Consume_Lifecycle(
	LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE& Lifecycle)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (m_HarnessInput.Lifecycles.empty())
		return false;
	Lifecycle = std::move(m_HarnessInput.Lifecycles.front());
	m_HarnessInput.Lifecycles.pop_front();
	return true;
#else
	return CNetworkManager::Get().Try_Consume_ValtanPatternFlowLifecycle(Lifecycle);
#endif
}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
void Client::CValtanPatternFlowService::Harness_Reset()
{
	m_Snapshot = {};
	m_PendingStart = {};
	m_CurrentRequest = {};
	m_iNextRequestSequence = 1u;
	m_iNextControlSequence = 1u;
	m_iPendingStopControlSequence = 0u;
	m_iPendingStopStartedAtMilliseconds = 0u;
	m_HarnessInput = {};
}
#endif
