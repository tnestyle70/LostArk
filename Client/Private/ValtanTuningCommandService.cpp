#include "ValtanTuningCommandService.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "Network/PacketWriter.h"
#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
#include "NetworkManager.h"
#endif

namespace
{
	constexpr uint64_t APPLY_TIMEOUT_MILLISECONDS = 10000u;

	bool Is_LowerSha256(const std::string_view Value)
	{
		return Value.size() == 64u &&
			std::all_of(Value.begin(), Value.end(), [](const char Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
	}

}

Client::CValtanTuningCommandService& Client::CValtanTuningCommandService::Get()
{
	static CValtanTuningCommandService Service;
	return Service;
}

bool Client::CValtanTuningCommandService::Has_PendingCommand() const
{
	return m_bApplyPending ||
		Read_RevisionObservation().bOtherTransactionPending;
}

void Client::CValtanTuningCommandService::
	Record_GameplaySourceActivationExpectation(
		const std::string_view strCandidateRevision,
		const std::string_view strApplyClass,
		const std::string_view strStatus)
{
	if (m_strGameplayCandidateRevision != strCandidateRevision ||
		m_strGameplayCandidateApplyClass != strApplyClass)
	{
		m_strQueuedGameplayCandidateRevision.clear();
		m_strQueuedGameplayCandidateApplyClass.clear();
		m_bQueuedGameplayCandidateWaitsForOwnedTerminal = false;
	}
	m_bGameplaySourceActivationObserved = true;
	m_strGameplayCandidateRevision = std::string(strCandidateRevision);
	m_strGameplayCandidateApplyClass = std::string(strApplyClass);
	m_strGameplayActivationStatus = std::string(strStatus);
	if (!m_strGameplayCandidateRevision.empty() &&
		!Is_LowerSha256(m_strGameplayCandidateRevision))
	{
		m_strGameplayActivationStatus =
			"The saved gameplay source produced an invalid candidate revision; Server activation cannot be confirmed.";
		m_strGameplayCandidateRevision.clear();
		m_strGameplayCandidateApplyClass.clear();
	}
}

bool Client::CValtanTuningCommandService::
	Queue_GameplaySourceCandidateAfterPending(
		const std::string_view strCandidateRevision,
		const std::string_view strApplyClass,
		std::string& strOutStatus)
{
	const std::string Candidate(strCandidateRevision);
	const std::string ApplyClass(strApplyClass);
	Update();
	if (!Is_LowerSha256(Candidate) || "HOT_RELOAD" != ApplyClass)
	{
		strOutStatus =
			"Only a valid immutable HOT_RELOAD candidate can wait behind another revision transaction.";
		return false;
	}
	if (!m_bGameplaySourceActivationObserved ||
		m_strGameplayCandidateRevision != Candidate ||
		m_strGameplayCandidateApplyClass != ApplyClass)
	{
		strOutStatus =
			"The deferred candidate is not the exact latest saved gameplay source expectation.";
		return false;
	}
	if (!Has_PendingCommand())
	{
		if (m_strQueuedGameplayCandidateRevision == Candidate)
		{
			m_strQueuedGameplayCandidateRevision.clear();
			m_strQueuedGameplayCandidateApplyClass.clear();
			m_bQueuedGameplayCandidateWaitsForOwnedTerminal = false;
		}
		return ApplyCandidate(Candidate, ApplyClass, strOutStatus);
	}
	if (m_bApplyPending && m_Snapshot.strCandidateRevision == Candidate)
	{
		strOutStatus =
			"The exact latest saved candidate is already the in-flight revision transaction.";
		m_strGameplayActivationStatus = strOutStatus;
		return true;
	}

	m_strQueuedGameplayCandidateRevision = Candidate;
	m_strQueuedGameplayCandidateApplyClass = ApplyClass;
	m_bQueuedGameplayCandidateWaitsForOwnedTerminal = m_bApplyPending;
	strOutStatus =
		"The latest saved candidate is queued behind the unresolved revision transaction and will submit after its exact terminal result.";
	m_strGameplayActivationStatus = strOutStatus;
	return true;
}

bool Client::CValtanTuningCommandService::
	Is_LatestGameplaySourceServerActive(std::string& strOutStatus) const
{
	if (!m_bGameplaySourceActivationObserved)
	{
		strOutStatus.clear();
		return true;
	}
	LostArk::Shared::GameplayDataRevision ignoredRevision{};
	return Try_GetLatestGameplaySourceServerActiveRevision(
		ignoredRevision, strOutStatus);
}

bool Client::CValtanTuningCommandService::
	Try_GetLatestGameplaySourceServerActiveRevision(
		LostArk::Shared::GameplayDataRevision& outRevision,
		std::string& strOutStatus) const
{
	using namespace LostArk::Shared;
	outRevision = {};
	if (!m_bGameplaySourceActivationObserved)
	{
		strOutStatus =
			"No saved gameplay source activation expectation is registered.";
		return false;
	}

	GameplayDataRevision CandidateRevision;
	if (m_strGameplayCandidateRevision.empty() ||
		!Try_Parse_GameplayDataRevision(
			m_strGameplayCandidateRevision, CandidateRevision))
	{
		strOutStatus =
			"Complete Play is blocked because the latest saved gameplay source has no admitted Product candidate.";
		if (!m_strGameplayActivationStatus.empty())
			strOutStatus += " " + m_strGameplayActivationStatus;
		return false;
	}

	const REVISION_OBSERVATION Observation = Read_RevisionObservation();
	if (Observation.bConnected &&
		Observation.ServerActiveRevision.Is_Valid() &&
		Observation.ServerActiveRevision == CandidateRevision)
	{
		outRevision = CandidateRevision;
		strOutStatus.clear();
		return true;
	}

	strOutStatus =
		"Complete Play is blocked until the latest saved gameplay candidate is the Server-active revision";
	if (!m_strGameplayCandidateApplyClass.empty())
		strOutStatus += " (apply class " +
			m_strGameplayCandidateApplyClass + ")";
	strOutStatus += ".";
	if (!m_strGameplayActivationStatus.empty())
		strOutStatus += " " + m_strGameplayActivationStatus;
	return false;
}

bool Client::CValtanTuningCommandService::ApplyCandidate(
	const std::string_view strCandidateRevision, const std::string_view strApplyClass,
	std::string& strOutStatus)
{
	// Callers may pass string_views into the current snapshot.
	const std::string Candidate(strCandidateRevision);
	const std::string ApplyClass(strApplyClass);
	Update();
	if (Has_PendingCommand())
	{
		strOutStatus = "The current Valtan command has no terminal result yet.";
		return false;
	}
	if (!Is_LowerSha256(Candidate) || ApplyClass != "HOT_RELOAD")
	{
		strOutStatus = ApplyClass == "ENCOUNTER_RESET" || ApplyClass == "SERVER_RESTART" ?
			"This candidate requires a controlled encounter reset or Server restart; Hot Reload was not requested." :
			"Apply requires a valid immutable HOT_RELOAD candidate.";
		return false;
	}
	const REVISION_OBSERVATION Observation = Read_RevisionObservation();
	if (Observation.bOtherTransactionPending)
	{
		strOutStatus = "Another Valtan revision transaction is pending; its candidate was not replaced.";
		return false;
	}
	if (m_Snapshot.strCandidateRevision != Candidate)
	{
		m_Snapshot = {};
		m_Snapshot.strCandidateRevision = Candidate;
	}
	m_Snapshot.strApplyClass = ApplyClass;
	m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
	return Submit_Candidate(Observation, strOutStatus);
}

bool Client::CValtanTuningCommandService::Submit_Candidate(
	const REVISION_OBSERVATION& Observation, std::string& strOutStatus)
{
	using namespace LostArk::Shared;
	auto Reject = [&](std::string Reason)
	{
		m_Snapshot.strStatus = std::move(Reason);
		strOutStatus = m_Snapshot.strStatus;
		return false;
	};
	Refresh_ActiveCandidate(Observation);
	if (m_Snapshot.bCandidateIsServerActive)
	{
		m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE;
		m_Snapshot.iConnectionGeneration = Observation.iConnectionGeneration;
		m_Snapshot.iWorldInboundGeneration = Observation.iWorldInboundGeneration;
		m_Snapshot.iRequestSequence = 0u;
		m_Snapshot.strStatus = "The Server already reports this candidate active. No new revision transaction was submitted.";
		strOutStatus = m_Snapshot.strStatus;
		return true;
	}
	if (!Is_ActivationEnabled())
		return Reject("Release Client does not initiate gameplay data revision transactions.");
	if (m_Snapshot.strApplyClass != "HOT_RELOAD")
		return Reject("Candidate publication succeeded, but its apply class requires a controlled reset or Server restart.");
	if (!Observation.bConnected || Observation.iConnectionGeneration == 0u ||
		Observation.iWorldInboundGeneration == 0u || !Observation.ServerActiveRevision.Is_Valid())
	{
		return Reject("Candidate published. Connect to the Server in Valtan Arena, then retry Apply; runtime application is not confirmed.");
	}
	if (Observation.bOtherTransactionPending)
		return Reject("Candidate published, but another revision transaction is pending. Retry Apply after it settles.");
	if (m_iNextRequestSequence == 0u)
		return Reject("Valtan revision request identities are exhausted; restart the Client before another Apply.");
	C2S_DATA_REVISION_PREPARE_REQUEST Request;
	Request.iTransactionSequence = m_iNextRequestSequence;
	Request.BaseRevision = Observation.ServerActiveRevision;
	if (!Try_Parse_GameplayDataRevision(m_Snapshot.strCandidateRevision, Request.CandidateRevision))
		return Reject("The published candidate revision is invalid.");
	Request.iRequiredPresentationLaneMask = GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
#if !defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	const CNetworkManager::PRESENTATION_CANDIDATE_PREFLIGHT_RESULT preflight =
		CNetworkManager::Get().Preflight_PresentationCandidate(
			Request.CandidateRevision,
			Request.iRequiredPresentationLaneMask,
			strOutStatus);
	if (CNetworkManager::PRESENTATION_CANDIDATE_PREFLIGHT_RESULT::
			CURRENT_GENERATION_READY != preflight)
	{
		return Reject(std::move(strOutStatus));
	}
#endif
	if (!Send_PrepareRequest(Request))
		return Reject("Candidate published, but the typed prepare request could not be sent. Runtime application is not confirmed.");
	m_iNextRequestSequence = (std::numeric_limits<uint32_t>::max)() == m_iNextRequestSequence ?
		0u : m_iNextRequestSequence + 1u;
	m_ApplyRequest = std::move(Request);
	m_bApplyPending = true;
	m_iApplyStartedAtMilliseconds = Now_Milliseconds();
	m_Snapshot.iRequestSequence = m_ApplyRequest.iTransactionSequence;
	m_Snapshot.iConnectionGeneration = Observation.iConnectionGeneration;
	m_Snapshot.iWorldInboundGeneration = Observation.iWorldInboundGeneration;
	m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING;
	m_Snapshot.strStatus = "Candidate published; waiting for the matching Server tick commit and Client presentation result.";
	strOutStatus = m_Snapshot.strStatus;
	return true;
}

void Client::CValtanTuningCommandService::Refresh_ActiveCandidate(
	const REVISION_OBSERVATION& Observation)
{
	using namespace LostArk::Shared;
	GameplayDataRevision Candidate;
	m_Snapshot.bCandidateIsServerActive = Observation.bConnected &&
		Observation.iConnectionGeneration != 0u && Observation.iWorldInboundGeneration != 0u &&
		Try_Parse_GameplayDataRevision(m_Snapshot.strCandidateRevision, Candidate) &&
		Candidate == Observation.ServerActiveRevision;
	const bool WasConfirmed = m_Snapshot.eState == VALTAN_TUNING_COMMAND_STATE::COMMITTED ||
		m_Snapshot.eState == VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE;
	if (!WasConfirmed)
		return;
	if (!m_Snapshot.bCandidateIsServerActive)
	{
		m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
		m_Snapshot.strStatus = "This candidate is not confirmed as the current Server revision. The previous Apply result is historical; retry Apply if needed.";
	}
	else if (m_Snapshot.iConnectionGeneration != Observation.iConnectionGeneration ||
		m_Snapshot.iWorldInboundGeneration != Observation.iWorldInboundGeneration)
	{
		m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE;
		m_Snapshot.iConnectionGeneration = Observation.iConnectionGeneration;
		m_Snapshot.iWorldInboundGeneration = Observation.iWorldInboundGeneration;
		m_Snapshot.iRequestSequence = 0u;
		m_Snapshot.strStatus = "The current Server world reports this candidate active. The previous transaction belonged to an earlier connection or world.";
	}
}

void Client::CValtanTuningCommandService::Update()
{
	using namespace LostArk::Shared;
	const REVISION_OBSERVATION Observation = Read_RevisionObservation();
	bool bOwnedApplySettledExactly = false;
	if (m_bApplyPending)
	{
		if (m_Snapshot.iConnectionGeneration != Observation.iConnectionGeneration ||
			m_Snapshot.iWorldInboundGeneration != Observation.iWorldInboundGeneration)
		{
			m_bApplyPending = false;
			m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
			m_Snapshot.strStatus = "Connection or world changed before the matching Apply result was observed. This does not cancel or confirm the Server transaction; retry Apply in the current world.";
		}
		else if (Observation.bConnected && Observation.bHasLatestResult &&
			!Observation.bOutstandingPrepareRequest &&
			Observation.iLatestTransactionSequence == m_ApplyRequest.iTransactionSequence &&
			Observation.LatestCandidateRevision == m_ApplyRequest.CandidateRevision)
		{
			if (Observation.eLatestResult == DATA_REVISION_RESULT::ABORTED)
			{
				m_bApplyPending = false;
				bOwnedApplySettledExactly = true;
				m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::FAILED;
				m_Snapshot.strStatus = "Server rejected the candidate; its previous revision remains active. " + Observation.strReason;
			}
			else if (Observation.eLatestResult == DATA_REVISION_RESULT::COMMITTED &&
				Observation.ServerActiveRevision == m_ApplyRequest.CandidateRevision)
			{
				m_bApplyPending = false;
				bOwnedApplySettledExactly = true;
				m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::COMMITTED;
				m_Snapshot.strStatus = "Server committed this candidate. A running Product sequence keeps its old definition; the next encounter/reset uses the saved order.";
			}
		}
		if (m_bApplyPending && (!Observation.bConnected ||
			Now_Milliseconds() - m_iApplyStartedAtMilliseconds >= APPLY_TIMEOUT_MILLISECONDS))
		{
			m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED;
			m_Snapshot.strStatus = "The Server Apply outcome is unconfirmed. No success or cancellation is assumed; another revision command is blocked.";
		}
	}
	Refresh_ActiveCandidate(Observation);
	Submit_QueuedGameplayCandidateAfterPending(
		Observation, bOwnedApplySettledExactly);
}

void Client::CValtanTuningCommandService::
	Submit_QueuedGameplayCandidateAfterPending(
		const REVISION_OBSERVATION& Observation,
		const bool bOwnedApplySettledExactly)
{
	if (m_strQueuedGameplayCandidateRevision.empty() || m_bApplyPending ||
		Observation.bOtherTransactionPending ||
		(m_bQueuedGameplayCandidateWaitsForOwnedTerminal &&
		 !bOwnedApplySettledExactly))
	{
		return;
	}

	const std::string Candidate = m_strQueuedGameplayCandidateRevision;
	const std::string ApplyClass = m_strQueuedGameplayCandidateApplyClass;
	m_strQueuedGameplayCandidateRevision.clear();
	m_strQueuedGameplayCandidateApplyClass.clear();
	m_bQueuedGameplayCandidateWaitsForOwnedTerminal = false;
	if (!Is_LowerSha256(Candidate) || "HOT_RELOAD" != ApplyClass ||
		m_strGameplayCandidateRevision != Candidate ||
		m_strGameplayCandidateApplyClass != ApplyClass)
	{
		m_strGameplayActivationStatus =
			"The deferred gameplay candidate became stale before submission and was not applied.";
		return;
	}

	if (m_Snapshot.strCandidateRevision != Candidate)
	{
		m_Snapshot = {};
		m_Snapshot.strCandidateRevision = Candidate;
	}
	m_Snapshot.strApplyClass = ApplyClass;
	m_Snapshot.eState = VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED;
	std::string Status;
	if (!Submit_Candidate(Observation, Status))
	{
		m_strGameplayActivationStatus =
			"The queued latest saved candidate could not start after the previous transaction settled: " +
			Status;
		return;
	}
	m_strGameplayActivationStatus = Status;
}

bool Client::CValtanTuningCommandService::Is_ActivationEnabled() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.bActivationEnabled;
#elif defined(_DEBUG)
	return true;
#else
	return false;
#endif
}

uint64_t Client::CValtanTuningCommandService::Now_Milliseconds() const
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	return m_HarnessInput.iNowMilliseconds;
#else
	return GetTickCount64();
#endif
}

Client::CValtanTuningCommandService::REVISION_OBSERVATION
Client::CValtanTuningCommandService::Read_RevisionObservation() const
{
	REVISION_OBSERVATION Observation;
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	Observation.bConnected = m_HarnessInput.bConnected;
	Observation.iConnectionGeneration = m_HarnessInput.iConnectionGeneration;
	Observation.iWorldInboundGeneration = m_HarnessInput.iWorldInboundGeneration;
	Observation.bOtherTransactionPending = m_HarnessInput.bOutstandingPrepareRequest ||
		m_HarnessInput.bStagedPresentationAlias || m_HarnessInput.bRejectedPrepareAwaitingAbort;
	Observation.bOutstandingPrepareRequest = m_HarnessInput.bOutstandingPrepareRequest;
	Observation.bHasLatestResult = m_HarnessInput.bHasLatestResult;
	Observation.iLatestTransactionSequence = m_HarnessInput.iLatestTransactionSequence;
	Observation.ServerActiveRevision = m_HarnessInput.ServerActiveRevision;
	Observation.LatestCandidateRevision = m_HarnessInput.LatestCandidateRevision;
	Observation.eLatestResult = m_HarnessInput.eLatestResult;
	Observation.strReason = m_HarnessInput.strLatestTransactionReason;
#else
	const CNetworkManager& Network = CNetworkManager::Get();
	const auto& State = Network.Get_GameplayRevisionState();
	Observation.bConnected = Network.Is_Connected();
	Observation.iConnectionGeneration = Network.Get_SessionDiagnosticSnapshot().iConnectionGeneration;
	Observation.iWorldInboundGeneration = Network.Get_WorldInboundGeneration();
	Observation.bOtherTransactionPending = State.hasOutstandingPrepareRequest ||
		State.hasStagedPresentationAlias || State.hasRejectedPrepareAwaitingAbort;
	Observation.bOutstandingPrepareRequest = State.hasOutstandingPrepareRequest;
	Observation.bHasLatestResult = State.hasLatestResult;
	Observation.iLatestTransactionSequence = State.iLatestTransactionSequence;
	Observation.ServerActiveRevision = State.ServerActiveRevision;
	Observation.LatestCandidateRevision = State.LatestCandidateRevision;
	Observation.eLatestResult = State.eLatestResult;
	Observation.strReason = State.strLatestTransactionReason;
#endif
	return Observation;
}

bool Client::CValtanTuningCommandService::Send_PrepareRequest(
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& Request)
{
#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	LostArk::Shared::CPacketWriter Writer;
	if (!LostArk::Shared::Write_Message(Writer, Request))
		return false;
	m_HarnessInput.SentRequests.push_back(Request);
	if (!m_HarnessInput.bSendSucceeds)
		return false;
	m_HarnessInput.bOutstandingPrepareRequest = true;
	return true;
#else
	return CNetworkManager::Get().Send_DataRevisionPrepareRequest(Request);
#endif
}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
void Client::CValtanTuningCommandService::Harness_Reset()
{
	m_Snapshot = {};
	m_ApplyRequest = {};
	m_bApplyPending = false;
	m_iApplyStartedAtMilliseconds = 0u;
	m_iNextRequestSequence = 1u;
	m_bGameplaySourceActivationObserved = false;
	m_strGameplayCandidateRevision.clear();
	m_strGameplayCandidateApplyClass.clear();
	m_strGameplayActivationStatus.clear();
	m_strQueuedGameplayCandidateRevision.clear();
	m_strQueuedGameplayCandidateApplyClass.clear();
	m_bQueuedGameplayCandidateWaitsForOwnedTerminal = false;
	m_HarnessInput = {};
}
#endif

const char* Client::Describe_ValtanTuningCommandState(const VALTAN_TUNING_COMMAND_STATE eState)
{
	switch (eState)
	{
	case VALTAN_TUNING_COMMAND_STATE::IDLE: return "IDLE";
	case VALTAN_TUNING_COMMAND_STATE::UNCONFIRMED: return "UNCONFIRMED";
	case VALTAN_TUNING_COMMAND_STATE::PUBLISHED_APPLY_NEEDED: return "PUBLISHED / APPLY NEEDED";
	case VALTAN_TUNING_COMMAND_STATE::APPLY_PENDING: return "APPLY PENDING";
	case VALTAN_TUNING_COMMAND_STATE::COMMITTED: return "COMMITTED";
	case VALTAN_TUNING_COMMAND_STATE::ALREADY_ACTIVE: return "ALREADY ACTIVE";
	case VALTAN_TUNING_COMMAND_STATE::FAILED: return "FAILED";
	default: return "UNKNOWN";
	}
}
