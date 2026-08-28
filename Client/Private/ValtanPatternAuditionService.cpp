#include "ValtanPatternAuditionService.h"
#include "ActionPresentationTimeline.h"

#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
#include "CombatHUDViewModel.h"
#include "NetworkManager.h"
#include "ValtanPatternFlowService.h"
#endif

#include <Windows.h>
#include <limits>
#include <utility>

namespace
{
	constexpr uint64_t VERDICT_TIMEOUT_MILLISECONDS = 5000u;
	constexpr uint64_t QUEUED_START_TIMEOUT_MILLISECONDS = 15000u;

	std::string Describe_Rejection(
		const LostArk::Shared::VALTAN_AUDITION_RESULT eResult,
		const std::string& strBossPlacementId)
	{
		using LostArk::Shared::VALTAN_AUDITION_RESULT;
		switch (eResult)
		{
		case VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD:
			return "Server Pattern Play is Debug-only; start the Debug Server.";
		case VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD:
			return "The active Server room is not Valtan Arena.";
		case VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS:
			return "No replicated Valtan exists at " + strBossPlacementId + ".";
		case VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD:
			return "The replicated Valtan is dead; respawn or re-enter before replaying.";
		case VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR:
			return "The Server encounter does not own this stable pattern ID.";
		case VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE:
			return "The Server rejected this pattern for the current boss state.";
		case VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED:
			return "The Server returned an invalid health-bar audition state.";
		case VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED:
			return "A living combat-ready player must be inside Valtan engage range.";
		case VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST:
			return "The Server rejected a stale or reused request identity.";
		case VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION:
			return "The predecessor audition is no longer current on the Server.";
		case VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER:
			return "This Client session does not own the Server audition.";
		case VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED:
			return "The Server Next reservation changed before this command arrived.";
		case VALTAN_AUDITION_RESULT::ARMED:
			return "The Server armed the request but did not queue the stable-ID pattern.";
		case VALTAN_AUDITION_RESULT::END:
		default:
			return "The Server returned an unexpected Pattern Play verdict.";
		}
	}

	std::string Short_Revision(const LostArk::Shared::GameplayDataRevision& Revision)
	{
		std::string Text = LostArk::Shared::Format_GameplayDataRevision(Revision);
		if (Text.size() > 12u)
			Text.resize(12u);
		return Text.empty() ? std::string("INVALID") : Text;
	}

	bool Exact_CommandResult(
		const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& Request,
		const LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& Result)
	{
		return Request.iRequestSequence == Result.iRequestSequence &&
			Request.eOperation == Result.eOperation &&
			Request.iTargetHealthBar == Result.iTargetHealthBar &&
			Request.strBossPlacementId == Result.strBossPlacementId &&
			Request.strPatternId == Result.strPatternId &&
			Request.iPredecessorRoomAuditionEpoch == Result.iPredecessorRoomAuditionEpoch &&
			Request.iPredecessorPatternSequence == Result.iPredecessorPatternSequence &&
			Request.iExpectedNextRequestSequence == Result.iExpectedNextRequestSequence;
	}

	bool Can_IntroduceOccurrence(
		const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE State)
	{
		using namespace LostArk::Shared;
		return VALTAN_AUDITION_LIFECYCLE_STATE::PENDING == State ||
			VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE == State;
	}
}

Client::CValtanPatternAuditionService& Client::CValtanPatternAuditionService::Get()
{
	static CValtanPatternAuditionService Instance;
	return Instance;
}

const char* Client::Describe_ValtanPatternAuditionState(
	const VALTAN_PATTERN_AUDITION_STATE eState)
{
	switch (eState)
	{
	case VALTAN_PATTERN_AUDITION_STATE::IDLE: return "IDLE";
	case VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING: return "REQUEST_PENDING";
	case VALTAN_PATTERN_AUDITION_STATE::QUEUED: return "QUEUED";
	case VALTAN_PATTERN_AUDITION_STATE::ACTIVE: return "ACTIVE";
	case VALTAN_PATTERN_AUDITION_STATE::COMPLETED: return "COMPLETED";
	case VALTAN_PATTERN_AUDITION_STATE::REJECTED: return "REJECTED";
	case VALTAN_PATTERN_AUDITION_STATE::ABORTED: return "ABORTED";
	case VALTAN_PATTERN_AUDITION_STATE::END:
	default: return "INVALID";
	}
}

const char* Client::Describe_ValtanNextPatternState(const VALTAN_NEXT_PATTERN_STATE eState)
{
	switch (eState)
	{
	case VALTAN_NEXT_PATTERN_STATE::NONE: return "NONE";
	case VALTAN_NEXT_PATTERN_STATE::RESERVED: return "RESERVED";
	case VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER: return "WAITING_FOR_PLAYER";
	case VALTAN_NEXT_PATTERN_STATE::START_PENDING: return "START_PENDING";
	case VALTAN_NEXT_PATTERN_STATE::REJECTED: return "REJECTED";
	case VALTAN_NEXT_PATTERN_STATE::ABORTED: return "ABORTED";
	case VALTAN_NEXT_PATTERN_STATE::END:
	default: return "INVALID";
	}
}

const char* Client::Describe_ValtanNextCommandState(const VALTAN_NEXT_COMMAND_STATE eState)
{
	switch (eState)
	{
	case VALTAN_NEXT_COMMAND_STATE::NONE: return "NONE";
	case VALTAN_NEXT_COMMAND_STATE::WAITING_VERDICT: return "WAITING_VERDICT";
	case VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED: return "UNCONFIRMED";
	case VALTAN_NEXT_COMMAND_STATE::END:
	default: return "INVALID";
	}
}

void Client::CValtanPatternAuditionService::Advance_RequestSequence()
{
	/* Zero is exhausted, never a wrap to a previously used wire identity. */
	m_iNextRequestSequence =
		(std::numeric_limits<uint32_t>::max)() == m_iNextRequestSequence ?
			0u : m_iNextRequestSequence + 1u;
}

bool Client::CValtanPatternAuditionService::Submit(
	const std::string_view strConsumerId,
	const std::string_view strBossPlacementId,
	const std::string_view strPatternId,
	std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	Update();
	if (strConsumerId.empty() || strBossPlacementId.empty() || strPatternId.empty())
	{
		strOutStatus = "Pattern Play requires stable consumer, boss placement, and pattern IDs.";
		return false;
	}
	if (Has_PlaybackOwnership() || Is_FlowInFlight())
	{
		strOutStatus = "Pattern Play is locked while an audition, Next command, or ordered Flow owns playback.";
		return false;
	}
	if (!Is_Connected())
	{
		strOutStatus = "Start and connect the Debug Server first.";
		return false;
	}
	if (0u == m_iNextRequestSequence)
	{
		strOutStatus = "Audition request identities are exhausted; restart the Client session.";
		return false;
	}
	C2S_VALTAN_AUDITION_REQUEST Request{};
	Request.iRequestSequence = m_iNextRequestSequence;
	Request.eOperation = VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID;
	Request.strBossPlacementId = strBossPlacementId;
	Request.strPatternId = strPatternId;
	if (!Send_Request(Request))
	{
		strOutStatus = "Could not send Server Pattern Play.";
		return false;
	}
	Advance_RequestSequence();
	m_Snapshot = {};
	m_NextSnapshot = {};
	m_NextCommand = {};
	m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING;
	m_Snapshot.iRequestSequence = Request.iRequestSequence;
	m_Snapshot.iWorldInboundGeneration = World_InboundGeneration();
	m_Snapshot.strConsumerId = strConsumerId;
	m_Snapshot.strBossPlacementId = strBossPlacementId;
	m_Snapshot.strPatternId = strPatternId;
	m_Snapshot.strStatus =
		"Waiting for the Server to reset replicated Valtan and queue the pattern.";
	m_iStateStartedAtMilliseconds = Now_Milliseconds();
	m_hasAuthoritativeLifecycle = false;
	strOutStatus = m_Snapshot.strStatus;
	return true;
}

bool Client::CValtanPatternAuditionService::Can_QueueNextPattern(
	const std::string_view strBossPlacementId,
	std::string& strOutStatus) const
{
	LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request{};
	return Prepare_NextCommand(strBossPlacementId, Request, strOutStatus);
}

bool Client::CValtanPatternAuditionService::Prepare_NextCommand(
	const std::string_view strBossPlacementId,
	LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& Request,
	std::string& strOutStatus) const
{
	using namespace LostArk::Shared;
	if (strBossPlacementId.empty())
	{
		strOutStatus = "Next Pattern requires a stable boss placement ID.";
		return false;
	}
	if (!Is_Connected() || 0u == m_iNextRequestSequence)
	{
		strOutStatus = !Is_Connected() ? "Start and connect the Debug Server first." :
			"Audition request identities are exhausted; restart the Client session.";
		return false;
	}
	if (Has_PendingNextCommand())
	{
		strOutStatus = "Resolve or retry the pending Next command before choosing another pattern.";
		return false;
	}
	if (Is_FlowStartPending())
	{
		strOutStatus = "Wait for the pending Flow start or restart before choosing Next Pattern.";
		return false;
	}

	uint32_t iEpoch = 0u;
	uint32_t iPredecessorSequence = 0u;
	uint32_t iNextToken = 0u;
	bool bLivePredecessor = false;
	if (m_NextSnapshot.Is_Live())
	{
		if (m_NextSnapshot.bReservationConsumed || 0u == m_NextSnapshot.iPredecessorPatternSequence)
		{
			strOutStatus = "Next is starting. Wait for its ACTIVE lifecycle before choosing another pattern.";
			return false;
		}
		if (0u == m_NextSnapshot.iRoomAuditionEpoch ||
			m_NextSnapshot.iWorldInboundGeneration != World_InboundGeneration() ||
			strBossPlacementId != m_NextSnapshot.strBossPlacementId)
		{
			strOutStatus = "The approved Next reservation does not belong to this boss and world session.";
			return false;
		}
		/* A live Product/Flow predecessor has no isolated current snapshot.
		   Its approved reservation still owns the full replacement CAS tuple. */
		iEpoch = m_NextSnapshot.iRoomAuditionEpoch;
		iPredecessorSequence = m_NextSnapshot.iPredecessorPatternSequence;
		iNextToken = m_NextSnapshot.iRequestSequence;
	}
	else
	{
		const bool bPredecessorReady =
			(VALTAN_PATTERN_AUDITION_STATE::QUEUED == m_Snapshot.eState ||
			 VALTAN_PATTERN_AUDITION_STATE::ACTIVE == m_Snapshot.eState ||
			 VALTAN_PATTERN_AUDITION_STATE::COMPLETED == m_Snapshot.eState) &&
			m_hasAuthoritativeLifecycle && 0u != m_Snapshot.iRoomAuditionEpoch &&
			0u != m_Snapshot.iObservedPatternSequence &&
			m_Snapshot.iWorldInboundGeneration == World_InboundGeneration() &&
			strBossPlacementId == m_Snapshot.strBossPlacementId;
		if (bPredecessorReady && !Is_FlowInFlight())
		{
			iEpoch = m_Snapshot.iRoomAuditionEpoch;
			iPredecessorSequence = m_Snapshot.iObservedPatternSequence;
		}
		else
		{
			if (m_Snapshot.Is_InFlight() && !Is_FlowInFlight())
			{
				strOutStatus = "Wait for the current isolated audition's authoritative lifecycle.";
				return false;
			}
			if (!Read_LivePatternSequence(iPredecessorSequence))
			{
				strOutStatus = "Next Pattern needs a living replicated Valtan in the active Server room.";
				return false;
			}
			bLivePredecessor = true;
		}
	}
	if ((std::numeric_limits<uint32_t>::max)() == iPredecessorSequence)
	{
		strOutStatus = "The predecessor pattern sequence is exhausted; start a new isolated audition.";
		return false;
	}
	Request.eOperation = bLivePredecessor ?
		VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID :
		VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID;
	Request.strBossPlacementId = strBossPlacementId;
	Request.iPredecessorRoomAuditionEpoch = iEpoch;
	Request.iPredecessorPatternSequence = iPredecessorSequence;
	Request.iExpectedNextRequestSequence = iNextToken;
	strOutStatus.clear();
	return true;
}

bool Client::CValtanPatternAuditionService::Queue_NextPattern(
	const std::string_view strConsumerId,
	const std::string_view strBossPlacementId,
	const std::string_view strPatternId,
	std::string& strOutStatus)
{
	Update();
	if (strConsumerId.empty() || strPatternId.empty())
	{
		strOutStatus = "Next Pattern requires stable consumer, boss placement, and pattern IDs.";
		return false;
	}
	LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request{};
	if (!Prepare_NextCommand(strBossPlacementId, Request, strOutStatus))
		return false;
	Request.strPatternId = strPatternId;
	return Send_NextCommand(std::move(Request), strConsumerId, strOutStatus);
}

bool Client::CValtanPatternAuditionService::Clear_NextPattern(std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	Update();
	if (!m_NextSnapshot.Is_Live() || m_NextSnapshot.bReservationConsumed ||
		0u == m_NextSnapshot.iPredecessorPatternSequence || Has_PendingNextCommand())
	{
		strOutStatus = m_NextSnapshot.Is_Live() &&
			(m_NextSnapshot.bReservationConsumed || 0u == m_NextSnapshot.iPredecessorPatternSequence) ?
			"The Server already consumed this reservation. The promoted pattern must become ACTIVE before another Next command." :
			"Cancel requires an approved Next reservation and no unresolved Next command.";
		return false;
	}
	C2S_VALTAN_AUDITION_REQUEST Request{};
	Request.eOperation = VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID;
	Request.strBossPlacementId = m_NextSnapshot.strBossPlacementId;
	Request.strPatternId = m_NextSnapshot.strPatternId;
	Request.iPredecessorRoomAuditionEpoch = m_NextSnapshot.iRoomAuditionEpoch;
	Request.iPredecessorPatternSequence = m_NextSnapshot.iPredecessorPatternSequence;
	Request.iExpectedNextRequestSequence = m_NextSnapshot.iRequestSequence;
	return Send_NextCommand(std::move(Request), m_NextSnapshot.strConsumerId, strOutStatus);
}

bool Client::CValtanPatternAuditionService::Send_NextCommand(
	LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request,
	const std::string_view strConsumerId,
	std::string& strOutStatus)
{
	if (!Is_Connected() || 0u == m_iNextRequestSequence)
	{
		strOutStatus = !Is_Connected() ? "Start and connect the Debug Server first." :
			"Audition request identities are exhausted; restart the Client session.";
		return false;
	}
	Request.iRequestSequence = m_iNextRequestSequence;
	if (!Send_Request(Request))
	{
		strOutStatus = "Could not send the Next command; current playback and the approved reservation were preserved.";
		return false;
	}
	Advance_RequestSequence();
	m_NextCommand = {};
	m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::WAITING_VERDICT;
	m_NextCommand.Request = std::move(Request);
	m_NextCommand.iWorldInboundGeneration = World_InboundGeneration();
	m_NextCommand.iSentAtMilliseconds = Now_Milliseconds();
	m_NextCommand.strConsumerId = strConsumerId;
	m_NextCommand.strStatus = "Waiting for the exact Server Next command verdict.";
	strOutStatus = m_NextCommand.strStatus;
	return true;
}

bool Client::CValtanPatternAuditionService::Retry_NextPatternCommand(std::string& strOutStatus)
{
	Update();
	if (VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED != m_NextCommand.eState)
	{
		strOutStatus = "Only an unconfirmed Next command can retry its original identity.";
		return false;
	}
	if (!Is_Connected() || !Send_Request(m_NextCommand.Request))
	{
		strOutStatus = "Could not retry the Next command; its unresolved identity was preserved.";
		return false;
	}
	m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::WAITING_VERDICT;
	m_NextCommand.iSentAtMilliseconds = Now_Milliseconds();
	m_NextCommand.strStatus = "Retried the exact Next command; waiting for the Server's original verdict.";
	strOutStatus = m_NextCommand.strStatus;
	return true;
}

void Client::CValtanPatternAuditionService::Accept_NextCommand(const uint32_t iRoomAuditionEpoch)
{
	const auto& Request = m_NextCommand.Request;
	const bool bAdoptsLivePredecessor =
		LostArk::Shared::VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID ==
		Request.eOperation;
	const auto PinnedRevision = m_NextSnapshot.Is_Live() ?
		m_NextSnapshot.PinnedDefinitionRevision :
		(bAdoptsLivePredecessor ? LostArk::Shared::GameplayDataRevision{} :
		 m_Snapshot.PinnedDefinitionRevision);
	const bool bPresentationAvailable = m_NextSnapshot.Is_Live() ?
		m_NextSnapshot.isPresentationRevisionAvailable :
		(!bAdoptsLivePredecessor && m_Snapshot.isPresentationRevisionAvailable);
	m_NextSnapshot = {};
	m_NextSnapshot.eState = VALTAN_NEXT_PATTERN_STATE::RESERVED;
	m_NextSnapshot.iRequestSequence = Request.iRequestSequence;
	m_NextSnapshot.iWorldInboundGeneration = m_NextCommand.iWorldInboundGeneration;
	m_NextSnapshot.iRoomAuditionEpoch = iRoomAuditionEpoch;
	m_NextSnapshot.iPredecessorPatternSequence = Request.iPredecessorPatternSequence;
	m_NextSnapshot.iExpectedPatternSequence = Request.iPredecessorPatternSequence + 1u;
	m_NextSnapshot.PinnedDefinitionRevision = PinnedRevision;
	m_NextSnapshot.isPresentationRevisionAvailable = bPresentationAvailable;
	m_NextSnapshot.strConsumerId = m_NextCommand.strConsumerId;
	m_NextSnapshot.strBossPlacementId = Request.strBossPlacementId;
	m_NextSnapshot.strPatternId = Request.strPatternId;
	m_NextSnapshot.strStatus = "Server reserved Next Pattern after the current audition's terminal stage.";
	m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::NONE;
	m_NextCommand.strStatus = "Server approved the Next reservation.";
}

void Client::CValtanPatternAuditionService::Apply_ServerResult(
	const LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& Result)
{
	using namespace LostArk::Shared;
	if (VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Result.eOperation)
	{
		if (VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING != m_Snapshot.eState ||
			Result.iRequestSequence != m_Snapshot.iRequestSequence ||
			Result.strBossPlacementId != m_Snapshot.strBossPlacementId ||
			Result.strPatternId != m_Snapshot.strPatternId)
			return;
		if (VALTAN_AUDITION_RESULT::QUEUED == Result.eResult ||
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == Result.eResult)
		{
			m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::QUEUED;
			m_Snapshot.strStatus = "Server queued the full fixed-tick pattern.";
			m_iStateStartedAtMilliseconds = Now_Milliseconds();
		}
		else
			Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::REJECTED,
				Describe_Rejection(Result.eResult, m_Snapshot.strBossPlacementId));
		return;
	}
	if (!Has_PendingNextCommand() || !Exact_CommandResult(m_NextCommand.Request, Result))
		return;
	if ((VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID == Result.eOperation ||
		 VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == Result.eOperation) &&
		VALTAN_AUDITION_RESULT::QUEUED == Result.eResult)
	{
		if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == Result.eOperation)
		{
			/* The verdict echoes epoch zero. Only the exact lifecycle supplies
			   the Server-issued chain epoch; keep retry ownership until then. */
			m_NextCommand.strStatus =
				"Server approved live Next; waiting for its authoritative reservation lifecycle.";
		}
		else
			Accept_NextCommand(Result.iPredecessorRoomAuditionEpoch);
		return;
	}
	if (VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID == Result.eOperation &&
		VALTAN_AUDITION_RESULT::CLEARED == Result.eResult)
	{
		if (m_NextSnapshot.iRequestSequence == Result.iExpectedNextRequestSequence &&
			m_NextSnapshot.iRoomAuditionEpoch == Result.iPredecessorRoomAuditionEpoch &&
			m_NextSnapshot.iPredecessorPatternSequence == Result.iPredecessorPatternSequence &&
			m_NextSnapshot.strPatternId == Result.strPatternId)
		{
			m_NextSnapshot = {};
			m_NextSnapshot.strStatus = "Server cleared the reserved Next Pattern.";
		}
		m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::NONE;
		m_NextCommand.strStatus = "Server confirmed cancellation of the exact Next reservation.";
		return;
	}
	if (VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == Result.eResult ||
		VALTAN_AUDITION_RESULT::ARMED == Result.eResult ||
		VALTAN_AUDITION_RESULT::QUEUED == Result.eResult ||
		VALTAN_AUDITION_RESULT::CLEARED == Result.eResult)
	{
		m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED;
		m_NextCommand.strStatus =
			"Next requires its original operation-specific verdict; retry the same unresolved request.";
		return;
	}
	/* A rejected replacement or clear never deletes the approved B snapshot. */
	m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::NONE;
	m_NextCommand.strStatus = Describe_Rejection(Result.eResult, Result.strBossPlacementId);
	if (!m_NextSnapshot.Is_Live() &&
		(VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID == Result.eOperation ||
		 VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == Result.eOperation))
	{
		m_NextSnapshot.eState = VALTAN_NEXT_PATTERN_STATE::REJECTED;
		m_NextSnapshot.strStatus = m_NextCommand.strStatus;
	}
}

bool Client::CValtanPatternAuditionService::Apply_NextLifecycle(
	const LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& Lifecycle)
{
	using namespace LostArk::Shared;
	const auto& Request = m_NextCommand.Request;
	const bool bLiveCandidate =
		VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == Request.eOperation;
	const GameplayDataRevision CandidatePinnedRevision = m_NextSnapshot.Is_Live() ?
		m_NextSnapshot.PinnedDefinitionRevision :
		(bLiveCandidate ? GameplayDataRevision{} : m_Snapshot.PinnedDefinitionRevision);
	const bool bCandidate = Has_PendingNextCommand() &&
		(VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID == Request.eOperation || bLiveCandidate) &&
		Lifecycle.iRequestSequence == Request.iRequestSequence &&
		0u != Lifecycle.iRoomAuditionEpoch &&
		(bLiveCandidate || Lifecycle.iRoomAuditionEpoch == Request.iPredecessorRoomAuditionEpoch) &&
		Lifecycle.iPatternSequence == Request.iPredecessorPatternSequence + 1u &&
		Lifecycle.strPatternId == Request.strPatternId &&
		Lifecycle.PinnedDefinitionRevision.Is_Valid() &&
		(!CandidatePinnedRevision.Is_Valid() ||
		 CandidatePinnedRevision == Lifecycle.PinnedDefinitionRevision);
	/* NEXT_RESERVED can overtake its verdict. It confirms only the exact
	   reservation request, epoch and expected sequence of this candidate. */
	if (bCandidate)
	{
		switch (Lifecycle.eState)
		{
		case VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED:
		case VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER:
		case VALTAN_AUDITION_LIFECYCLE_STATE::PENDING:
		case VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE:
		case VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED:
		case VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED:
			Accept_NextCommand(Lifecycle.iRoomAuditionEpoch);
			break;
		default: return false;
		}
	}
	if (!m_NextSnapshot.Is_Live() ||
		Lifecycle.iRequestSequence != m_NextSnapshot.iRequestSequence ||
		Lifecycle.iRoomAuditionEpoch != m_NextSnapshot.iRoomAuditionEpoch ||
		Lifecycle.iPatternSequence != m_NextSnapshot.iExpectedPatternSequence ||
		Lifecycle.strPatternId != m_NextSnapshot.strPatternId ||
		!Lifecycle.PinnedDefinitionRevision.Is_Valid() ||
		(m_NextSnapshot.PinnedDefinitionRevision.Is_Valid() &&
		 m_NextSnapshot.PinnedDefinitionRevision != Lifecycle.PinnedDefinitionRevision))
		return false;

	m_hasAuthoritativeLifecycle = true;
	m_NextSnapshot.PinnedDefinitionRevision = Lifecycle.PinnedDefinitionRevision;
	m_NextSnapshot.isPresentationRevisionAvailable =
		Is_PresentationAvailable(Lifecycle.PinnedDefinitionRevision);
	switch (Lifecycle.eState)
	{
	case VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED:
		/* A replayed reservation must not regress WAITING or START_PENDING. */
		if (VALTAN_NEXT_PATTERN_STATE::RESERVED == m_NextSnapshot.eState)
			m_NextSnapshot.strStatus = "Server owns this Next reservation; current playback is unchanged.";
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER:
		m_NextSnapshot.eState = VALTAN_NEXT_PATTERN_STATE::WAITING_FOR_PLAYER;
		m_NextSnapshot.strStatus = m_NextSnapshot.bReservationConsumed ?
			"Next was promoted, but its target disappeared before starting. Waiting for a living, engaged player; the reservation was already consumed." :
			"Current pattern ended. Next is waiting for a living, engaged player; its reservation can still be replaced or cancelled.";
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::PENDING:
		m_NextSnapshot.eState = VALTAN_NEXT_PATTERN_STATE::START_PENDING;
		m_NextSnapshot.bReservationConsumed = true;
		m_NextSnapshot.strStatus = "Server selected Next without resetting the boss or arena; waiting for ACTIVE.";
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE:
		Promote_NextPattern();
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED:
		/* A terminal receipt remains authoritative if ACTIVE was not observed
		   by this main-thread drain. Preserve the completed Next identity. */
		Promote_NextPattern();
		Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::COMPLETED,
			"Server lifecycle COMPLETED for the reserved pattern.");
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED:
		m_NextSnapshot.eState = VALTAN_NEXT_PATTERN_STATE::ABORTED;
		m_NextSnapshot.strStatus = "Server lifecycle ABORTED: " + Lifecycle.strReason;
		break;
	default: return false;
	}
	return true;
}

void Client::CValtanPatternAuditionService::Promote_NextPattern()
{
	m_Snapshot = {};
	m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::ACTIVE;
	m_Snapshot.iRequestSequence = m_NextSnapshot.iRequestSequence;
	m_Snapshot.iWorldInboundGeneration = m_NextSnapshot.iWorldInboundGeneration;
	m_Snapshot.iRoomAuditionEpoch = m_NextSnapshot.iRoomAuditionEpoch;
	m_Snapshot.iObservedPatternSequence = m_NextSnapshot.iExpectedPatternSequence;
	m_Snapshot.PinnedDefinitionRevision = m_NextSnapshot.PinnedDefinitionRevision;
	m_Snapshot.isPresentationRevisionAvailable = m_NextSnapshot.isPresentationRevisionAvailable;
	m_Snapshot.strConsumerId = m_NextSnapshot.strConsumerId;
	m_Snapshot.strBossPlacementId = m_NextSnapshot.strBossPlacementId;
	m_Snapshot.strPatternId = m_NextSnapshot.strPatternId;
	m_Snapshot.strStatus = m_Snapshot.isPresentationRevisionAvailable ?
		"Server lifecycle ACTIVE for Next Pattern; the current arena state was preserved." :
		"Server lifecycle ACTIVE for Next Pattern; unavailable presentation was isolated.";
	m_NextSnapshot = {};
	m_hasAuthoritativeLifecycle = true;
	m_iStateStartedAtMilliseconds = Now_Milliseconds();
}

void Client::CValtanPatternAuditionService::Apply_ServerLifecycle(
	const LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& Lifecycle)
{
	using namespace LostArk::Shared;
	if (Apply_NextLifecycle(Lifecycle))
		return;
	const bool bCompletedAbort =
		VALTAN_PATTERN_AUDITION_STATE::COMPLETED == m_Snapshot.eState &&
		VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED == Lifecycle.eState;
	const bool bHasOccurrence = m_hasAuthoritativeLifecycle &&
		0u != m_Snapshot.iObservedPatternSequence;
	const bool bSameOccurrence = bHasOccurrence &&
		m_Snapshot.iObservedPatternSequence == Lifecycle.iPatternSequence;
	const bool bNewOccurrence = bHasOccurrence && !bSameOccurrence &&
		Can_IntroduceOccurrence(Lifecycle.eState) &&
		CActionPresentationTimeline::Is_ForwardTick(
			Lifecycle.iPatternSequence, m_Snapshot.iObservedPatternSequence);
	if ((!m_Snapshot.Is_InFlight() && !bCompletedAbort) ||
		Lifecycle.iRequestSequence != m_Snapshot.iRequestSequence ||
		Lifecycle.strPatternId != m_Snapshot.strPatternId ||
		0u == Lifecycle.iRoomAuditionEpoch || 0u == Lifecycle.iPatternSequence ||
		!Lifecycle.PinnedDefinitionRevision.Is_Valid() ||
		(0u != m_Snapshot.iRoomAuditionEpoch &&
			m_Snapshot.iRoomAuditionEpoch != Lifecycle.iRoomAuditionEpoch) ||
		(bHasOccurrence && m_Snapshot.PinnedDefinitionRevision !=
			Lifecycle.PinnedDefinitionRevision) ||
		(bHasOccurrence && !bSameOccurrence && !bNewOccurrence))
		return;
	if (VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED == Lifecycle.eState ||
		VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER == Lifecycle.eState)
		return;

	m_hasAuthoritativeLifecycle = true;
	m_Snapshot.iRoomAuditionEpoch = Lifecycle.iRoomAuditionEpoch;
	m_Snapshot.iObservedPatternSequence = Lifecycle.iPatternSequence;
	m_Snapshot.PinnedDefinitionRevision = Lifecycle.PinnedDefinitionRevision;
	m_Snapshot.isPresentationRevisionAvailable =
		Is_PresentationAvailable(Lifecycle.PinnedDefinitionRevision);
	const std::string Revision = Short_Revision(Lifecycle.PinnedDefinitionRevision);
	switch (Lifecycle.eState)
	{
	case VALTAN_AUDITION_LIFECYCLE_STATE::PENDING:
		if (bNewOccurrence ||
			VALTAN_PATTERN_AUDITION_STATE::ACTIVE != m_Snapshot.eState)
		{
			m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::QUEUED;
			m_Snapshot.strStatus = "Server lifecycle PENDING in room epoch " +
				std::to_string(Lifecycle.iRoomAuditionEpoch) + " (definition " + Revision + ").";
			m_iStateStartedAtMilliseconds = Now_Milliseconds();
		}
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE:
		m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::ACTIVE;
		m_Snapshot.strStatus = m_Snapshot.isPresentationRevisionAvailable ?
			"Server lifecycle ACTIVE at pattern sequence " + std::to_string(Lifecycle.iPatternSequence) +
				" (definition " + Revision + ")." :
			"Server lifecycle ACTIVE, but Client presentation revision " + Revision + " is unavailable and was isolated.";
		m_iStateStartedAtMilliseconds = Now_Milliseconds();
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED:
		Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::COMPLETED,
			"Server lifecycle COMPLETED for pattern sequence " + std::to_string(Lifecycle.iPatternSequence) + ".");
		break;
	case VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED:
		Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"Server lifecycle ABORTED: " + Lifecycle.strReason);
		break;
	default: break;
	}
}

void Client::CValtanPatternAuditionService::Set_Terminal(
	const VALTAN_PATTERN_AUDITION_STATE eState, std::string strStatus)
{
	m_Snapshot.eState = eState;
	m_Snapshot.strStatus = std::move(strStatus);
	m_iStateStartedAtMilliseconds = Now_Milliseconds();
}

void Client::CValtanPatternAuditionService::Observe_Boss(
	const bool bBossValid, const std::string_view strPatternId,
	const uint32_t iPatternSequence)
{
	if (m_hasAuthoritativeLifecycle)
		return;
	if (VALTAN_PATTERN_AUDITION_STATE::QUEUED == m_Snapshot.eState &&
		bBossValid && strPatternId == m_Snapshot.strPatternId)
	{
		m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::ACTIVE;
		m_Snapshot.iObservedPatternSequence = iPatternSequence;
		m_Snapshot.strStatus = "The replicated Server pattern is active on the current boss snapshot.";
		m_iStateStartedAtMilliseconds = Now_Milliseconds();
		return;
	}
	if (VALTAN_PATTERN_AUDITION_STATE::ACTIVE == m_Snapshot.eState && bBossValid &&
		(strPatternId != m_Snapshot.strPatternId ||
			iPatternSequence != m_Snapshot.iObservedPatternSequence))
	{
		Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::COMPLETED,
			"The replicated boss advanced beyond the auditioned pattern.");
	}
}

void Client::CValtanPatternAuditionService::Abort_WorldSession(const std::string& strReason)
{
	if (0u != m_Snapshot.iRequestSequence)
		Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::ABORTED, strReason);
	if (m_NextSnapshot.Is_Live())
	{
		m_NextSnapshot.eState = VALTAN_NEXT_PATTERN_STATE::ABORTED;
		m_NextSnapshot.strStatus = strReason;
	}
	if (Has_PendingNextCommand())
	{
		m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::NONE;
		m_NextCommand.strStatus = strReason;
	}
}

void Client::CValtanPatternAuditionService::Update()
{
	using namespace LostArk::Shared;
	const uint64_t iGeneration = World_InboundGeneration();
	const bool bWorldChanged =
		((m_Snapshot.Is_InFlight() || VALTAN_PATTERN_AUDITION_STATE::COMPLETED == m_Snapshot.eState) &&
			m_Snapshot.iWorldInboundGeneration != iGeneration) ||
		(m_NextSnapshot.Is_Live() && m_NextSnapshot.iWorldInboundGeneration != iGeneration) ||
		(Has_PendingNextCommand() && m_NextCommand.iWorldInboundGeneration != iGeneration);
	/* Invalidate before draining. A completed hold cannot adopt packets from
	   a previous connection, level, or world inbound generation. */
	if (!Is_Connected() || bWorldChanged)
	{
		Abort_WorldSession(bWorldChanged ?
			"Pattern Play and Next were cancelled because the Client world session changed." :
			"The Server disconnected; current playback and Next ownership were cancelled.");
		S2C_VALTAN_AUDITION_RESULT DiscardResult{};
		while (Consume_Result(DiscardResult)) {}
		S2C_VALTAN_AUDITION_LIFECYCLE DiscardLifecycle{};
		while (Consume_Lifecycle(DiscardLifecycle)) {}
		return;
	}
	S2C_VALTAN_AUDITION_RESULT Result{};
	while (Consume_Result(Result))
		Apply_ServerResult(Result);
	S2C_VALTAN_AUDITION_LIFECYCLE Lifecycle{};
	while (Consume_Lifecycle(Lifecycle))
		Apply_ServerLifecycle(Lifecycle);

	/* The versioned server lifecycle is the authority. HUD inference is only
	   the original Play fallback; it never promotes a Next reservation. */
#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (!m_hasAuthoritativeLifecycle)
	{
		const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
		Observe_Boss(Boss.isValid,
			Boss.isValid ? std::string_view(Boss.strPatternId) : std::string_view{},
			Boss.isValid ? Boss.iPatternSequence : 0u);
	}
#endif
	const uint64_t iElapsed = Now_Milliseconds() - m_iStateStartedAtMilliseconds;
	if (VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING == m_Snapshot.eState &&
		iElapsed > VERDICT_TIMEOUT_MILLISECONDS)
	{
		Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"Timed out waiting for the Server Pattern Play verdict.");
	}
	else if (VALTAN_PATTERN_AUDITION_STATE::QUEUED == m_Snapshot.eState &&
		iElapsed > QUEUED_START_TIMEOUT_MILLISECONDS)
	{
		Set_Terminal(VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"The queued pattern did not appear in a replicated boss snapshot in time.");
	}
	/* Reservation, player-wait and start-pending do not use Play's 15s timer.
	   A 5s control timeout retains ownership and the exact retry payload. */
	if (VALTAN_NEXT_COMMAND_STATE::WAITING_VERDICT == m_NextCommand.eState &&
		Now_Milliseconds() - m_NextCommand.iSentAtMilliseconds > VERDICT_TIMEOUT_MILLISECONDS)
	{
		m_NextCommand.eState = VALTAN_NEXT_COMMAND_STATE::UNCONFIRMED;
		m_NextCommand.strStatus =
			"Next command is unconfirmed. Retry the same request; approved reservation and playback remain unchanged.";
	}
}

uint64_t Client::CValtanPatternAuditionService::Now_Milliseconds() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.iNowMilliseconds;
#else
	return GetTickCount64();
#endif
}

bool Client::CValtanPatternAuditionService::Is_Connected() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.bConnected;
#else
	return CNetworkManager::Get().Is_Connected();
#endif
}

uint64_t Client::CValtanPatternAuditionService::World_InboundGeneration() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.iWorldInboundGeneration;
#else
	return CNetworkManager::Get().Get_WorldInboundGeneration();
#endif
}

bool Client::CValtanPatternAuditionService::Is_FlowInFlight() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.bFlowInFlight;
#else
	return CValtanPatternFlowService::Get().Has_PlaybackOwnership();
#endif
}

bool Client::CValtanPatternAuditionService::Is_FlowStartPending() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.bFlowStartPending;
#else
	return CValtanPatternFlowService::Get().Has_PendingStart();
#endif
}

bool Client::CValtanPatternAuditionService::Read_LivePatternSequence(uint32_t& iOutPatternSequence) const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (!m_HarnessInput.bLiveBossValid || !m_HarnessInput.bLiveBossAlive)
		return false;
	iOutPatternSequence = m_HarnessInput.iLivePatternSequence;
#else
	const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!Boss.isValid || "BOSS_VALTAN" != Boss.strArchetypeId || 0u == Boss.iCurrentHp)
		return false;
	iOutPatternSequence = Boss.iPatternSequence;
#endif
	return true;
}

bool Client::CValtanPatternAuditionService::Is_PresentationAvailable(
	const LostArk::Shared::GameplayDataRevision& Revision) const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	(void)Revision;
	return m_HarnessInput.bPresentationAvailable;
#else
	return CNetworkManager::Get().Is_PresentationRevisionAvailable(Revision);
#endif
}

bool Client::CValtanPatternAuditionService::Send_Request(
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& Request)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (!m_HarnessInput.bConnected || !m_HarnessInput.bSendSucceeds)
		return false;
	m_HarnessInput.SentRequests.push_back(Request);
	return true;
#else
	if (LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID == Request.eOperation)
		return CNetworkManager::Get().Send_ValtanPatternAuditionById(
			Request.iRequestSequence, Request.strBossPlacementId, Request.strPatternId);
	return CNetworkManager::Get().Send_ValtanNextPatternCommand(Request);
#endif
}

bool Client::CValtanPatternAuditionService::Consume_Result(
	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT& Result)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (m_HarnessInput.Results.empty())
		return false;
	Result = std::move(m_HarnessInput.Results.front());
	m_HarnessInput.Results.pop_front();
	return true;
#else
	return CNetworkManager::Get().Try_Consume_ValtanPatternAuditionByIdResult(Result);
#endif
}

bool Client::CValtanPatternAuditionService::Consume_Lifecycle(
	LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE& Lifecycle)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	if (m_HarnessInput.Lifecycles.empty())
		return false;
	Lifecycle = std::move(m_HarnessInput.Lifecycles.front());
	m_HarnessInput.Lifecycles.pop_front();
	return true;
#else
	return CNetworkManager::Get().Try_Consume_ValtanAuditionLifecycle(Lifecycle);
#endif
}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
void Client::CValtanPatternAuditionService::Harness_Reset()
{
	m_Snapshot = {};
	m_NextSnapshot = {};
	m_NextCommand = {};
	m_iNextRequestSequence = 1u;
	m_HarnessInput = {};
	m_iStateStartedAtMilliseconds = Now_Milliseconds();
	m_hasAuthoritativeLifecycle = false;
}

void Client::CValtanPatternAuditionService::Harness_ObserveBoss(
	const bool bBossValid, const std::string_view strPatternId,
	const uint32_t iPatternSequence)
{
	Observe_Boss(bBossValid, strPatternId, iPatternSequence);
}
#endif
