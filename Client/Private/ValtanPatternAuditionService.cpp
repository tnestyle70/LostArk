#include "ValtanPatternAuditionService.h"

#include "CombatHUDViewModel.h"
#include "NetworkManager.h"
#include "Network/PacketMessages.h"

#include <Windows.h>
#include <limits>

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
		case VALTAN_AUDITION_RESULT::ARMED:
			return "The Server armed the request but did not queue the stable-ID pattern.";
		case VALTAN_AUDITION_RESULT::END:
		default:
			return "The Server returned an unknown Pattern Play verdict.";
		}
	}

	std::string Short_Revision(
		const LostArk::Shared::GameplayDataRevision& Revision)
	{
		std::string Text =
			LostArk::Shared::Format_GameplayDataRevision(Revision);
		if (Text.size() > 12u)
			Text.resize(12u);
		return Text.empty() ? std::string("INVALID") : Text;
	}
}

Client::CValtanPatternAuditionService&
Client::CValtanPatternAuditionService::Get()
{
	static CValtanPatternAuditionService Instance;
	return Instance;
}

const char_t* Client::Describe_ValtanPatternAuditionState(
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

bool_t Client::CValtanPatternAuditionService::Submit(
	const std::string_view strConsumerId,
	const std::string_view strBossPlacementId,
	const std::string_view strPatternId,
	std::string& strOutStatus)
{
	Update();
	if (strConsumerId.empty() || strBossPlacementId.empty() || strPatternId.empty())
	{
		strOutStatus = "Pattern Play requires stable consumer, boss placement, and pattern IDs.";
		return false;
	}
	if (m_Snapshot.Is_InFlight())
	{
		strOutStatus = "Pattern Play is already " +
			std::string(Describe_ValtanPatternAuditionState(m_Snapshot.eState)) +
			" for " + m_Snapshot.strPatternId + " (owner " +
			m_Snapshot.strConsumerId + ").";
		return false;
	}
	if (!CNetworkManager::Get().Is_Connected())
	{
		strOutStatus = "Start and connect the Debug Server first.";
		return false;
	}

	const uint32_t iSequence = 0u == m_iNextRequestSequence ?
		1u : m_iNextRequestSequence;
	if (!CNetworkManager::Get().Send_ValtanPatternAuditionById(
			iSequence, strBossPlacementId, strPatternId))
	{
		strOutStatus = "Could not send Server Pattern Play.";
		return false;
	}
	m_iNextRequestSequence =
		(std::numeric_limits<uint32_t>::max)() == iSequence ?
			1u : iSequence + 1u;
	m_Snapshot = {};
	m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING;
	m_Snapshot.iRequestSequence = iSequence;
	m_Snapshot.iWorldInboundGeneration =
		CNetworkManager::Get().Get_WorldInboundGeneration();
	m_Snapshot.strConsumerId = strConsumerId;
	m_Snapshot.strBossPlacementId = strBossPlacementId;
	m_Snapshot.strPatternId = strPatternId;
	m_Snapshot.strStatus =
		"Waiting for the Server to reset replicated Valtan and queue the pattern.";
	m_iStateStartedAtMilliseconds = GetTickCount64();
	m_hasAuthoritativeLifecycle = false;
	strOutStatus = m_Snapshot.strStatus;
	return true;
}

void Client::CValtanPatternAuditionService::Set_Terminal(
	const VALTAN_PATTERN_AUDITION_STATE eState,
	std::string strStatus)
{
	m_Snapshot.eState = eState;
	m_Snapshot.strStatus = std::move(strStatus);
	m_iStateStartedAtMilliseconds = GetTickCount64();
}

void Client::CValtanPatternAuditionService::Observe_Boss(
	const bool_t bBossValid,
	const std::string_view strPatternId,
	const uint32_t iPatternSequence)
{
	if (VALTAN_PATTERN_AUDITION_STATE::QUEUED == m_Snapshot.eState &&
		bBossValid && strPatternId == m_Snapshot.strPatternId)
	{
		m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::ACTIVE;
		m_Snapshot.iObservedPatternSequence = iPatternSequence;
		m_Snapshot.strStatus =
			"The replicated Server pattern is active on the current boss snapshot.";
		m_iStateStartedAtMilliseconds = GetTickCount64();
		return;
	}
	if (VALTAN_PATTERN_AUDITION_STATE::ACTIVE != m_Snapshot.eState)
		return;
	if (!bBossValid)
		return;
	if (strPatternId != m_Snapshot.strPatternId ||
		iPatternSequence != m_Snapshot.iObservedPatternSequence)
	{
		Set_Terminal(
			VALTAN_PATTERN_AUDITION_STATE::COMPLETED,
			"The replicated boss advanced beyond the auditioned pattern.");
	}
}

void Client::CValtanPatternAuditionService::Update()
{
	using namespace LostArk::Shared;
	if (m_Snapshot.Is_InFlight() &&
		m_Snapshot.iWorldInboundGeneration !=
			CNetworkManager::Get().Get_WorldInboundGeneration())
	{
		Set_Terminal(
			VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"Pattern Play was cancelled because the Client world session changed.");
	}

	S2C_VALTAN_AUDITION_RESULT Result{};
	while (CNetworkManager::Get().Try_Consume_ValtanPatternAuditionByIdResult(Result))
	{
		if (VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING != m_Snapshot.eState)
			continue;
		if (VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID != Result.eOperation ||
			Result.iRequestSequence != m_Snapshot.iRequestSequence ||
			Result.strBossPlacementId != m_Snapshot.strBossPlacementId ||
			Result.strPatternId != m_Snapshot.strPatternId)
		{
			m_Snapshot.strStatus =
				"Ignored a mismatched Server verdict; waiting for the exact request identity.";
			continue;
		}
		if (VALTAN_AUDITION_RESULT::QUEUED == Result.eResult ||
			VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED == Result.eResult)
		{
			m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::QUEUED;
			m_Snapshot.strStatus =
				VALTAN_AUDITION_RESULT::QUEUED == Result.eResult ?
				"Server queued the full fixed-tick pattern." :
				"Server already accepted this exact request; no duplicate was queued.";
			m_iStateStartedAtMilliseconds = GetTickCount64();
		}
		else
		{
			Set_Terminal(
				VALTAN_PATTERN_AUDITION_STATE::REJECTED,
				Describe_Rejection(Result.eResult, m_Snapshot.strBossPlacementId));
		}
	}

	S2C_VALTAN_AUDITION_LIFECYCLE Lifecycle{};
	while (CNetworkManager::Get().Try_Consume_ValtanAuditionLifecycle(Lifecycle))
	{
		if (Lifecycle.iRequestSequence != m_Snapshot.iRequestSequence ||
			Lifecycle.strPatternId != m_Snapshot.strPatternId)
		{
			if (m_Snapshot.Is_InFlight())
			{
				m_Snapshot.strStatus =
					"Ignored a lifecycle event with a different stable request identity.";
			}
			continue;
		}
		if (0u != m_Snapshot.iRoomAuditionEpoch &&
			m_Snapshot.iRoomAuditionEpoch != Lifecycle.iRoomAuditionEpoch)
		{
			Set_Terminal(
				VALTAN_PATTERN_AUDITION_STATE::ABORTED,
				"Server audition epoch changed before this request reached a terminal state.");
			continue;
		}

		m_hasAuthoritativeLifecycle = true;
		m_Snapshot.iRoomAuditionEpoch = Lifecycle.iRoomAuditionEpoch;
		m_Snapshot.iObservedPatternSequence = Lifecycle.iPatternSequence;
		m_Snapshot.PinnedDefinitionRevision =
			Lifecycle.PinnedDefinitionRevision;
		m_Snapshot.isPresentationRevisionAvailable =
			CNetworkManager::Get().Is_PresentationRevisionAvailable(
				Lifecycle.PinnedDefinitionRevision);
		const std::string Revision =
			Short_Revision(Lifecycle.PinnedDefinitionRevision);

		switch (Lifecycle.eState)
		{
		case VALTAN_AUDITION_LIFECYCLE_STATE::PENDING:
			if (m_Snapshot.Is_InFlight())
			{
				m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::QUEUED;
				m_Snapshot.strStatus =
					"Server lifecycle PENDING in room epoch " +
					std::to_string(Lifecycle.iRoomAuditionEpoch) +
					" (definition " + Revision + ").";
				m_iStateStartedAtMilliseconds = GetTickCount64();
			}
			break;
		case VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE:
			if (m_Snapshot.Is_InFlight() ||
				VALTAN_PATTERN_AUDITION_STATE::ACTIVE == m_Snapshot.eState)
			{
				m_Snapshot.eState = VALTAN_PATTERN_AUDITION_STATE::ACTIVE;
				m_Snapshot.strStatus =
					m_Snapshot.isPresentationRevisionAvailable ?
						"Server lifecycle ACTIVE at pattern sequence " +
						std::to_string(Lifecycle.iPatternSequence) +
						" (definition " + Revision + ")." :
						"Server lifecycle ACTIVE, but Client presentation revision " +
						Revision + " is unavailable and was isolated.";
				m_iStateStartedAtMilliseconds = GetTickCount64();
			}
			break;
		case VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED:
			if (m_Snapshot.Is_InFlight() ||
				VALTAN_PATTERN_AUDITION_STATE::COMPLETED == m_Snapshot.eState)
			{
				Set_Terminal(
					VALTAN_PATTERN_AUDITION_STATE::COMPLETED,
					m_Snapshot.isPresentationRevisionAvailable ?
						"Server lifecycle COMPLETED for pattern sequence " +
						std::to_string(Lifecycle.iPatternSequence) + "." :
						"Server lifecycle COMPLETED; its unavailable presentation revision " +
						Revision + " remained isolated.");
			}
			break;
		case VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED:
			if (m_Snapshot.Is_InFlight() ||
				VALTAN_PATTERN_AUDITION_STATE::ABORTED == m_Snapshot.eState)
			{
				Set_Terminal(
					VALTAN_PATTERN_AUDITION_STATE::ABORTED,
					"Server lifecycle ABORTED: " + Lifecycle.strReason);
			}
			break;
		case VALTAN_AUDITION_LIFECYCLE_STATE::END:
		default:
			break;
		}
	}

	if (m_Snapshot.Is_InFlight() && !CNetworkManager::Get().Is_Connected())
	{
		Set_Terminal(
			VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"The Server disconnected during Pattern Play.");
		return;
	}

	/* The versioned server lifecycle is the authority.  HUD inference remains only as a
	   compatibility fallback until the first exact lifecycle event arrives; it
	   must not race an ACTIVE frame that precedes its matching world snapshot. */
	if (!m_hasAuthoritativeLifecycle)
	{
		const HUD_BOSS_STATE& Boss = CCombatHUDViewModel::Get().Get_Boss();
		Observe_Boss(
			Boss.isValid,
			Boss.isValid ? std::string_view(Boss.strPatternId) :
				std::string_view{},
			Boss.isValid ? Boss.iPatternSequence : 0u);
	}

	const uint64_t iElapsed = GetTickCount64() - m_iStateStartedAtMilliseconds;
	if (VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING == m_Snapshot.eState &&
		iElapsed > VERDICT_TIMEOUT_MILLISECONDS)
	{
		Set_Terminal(
			VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"Timed out waiting for the Server Pattern Play verdict.");
	}
	else if (VALTAN_PATTERN_AUDITION_STATE::QUEUED == m_Snapshot.eState &&
		iElapsed > QUEUED_START_TIMEOUT_MILLISECONDS)
	{
		Set_Terminal(
			VALTAN_PATTERN_AUDITION_STATE::ABORTED,
			"The queued pattern did not appear in a replicated boss snapshot in time.");
	}
}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
void Client::CValtanPatternAuditionService::Harness_Reset()
{
	m_Snapshot = {};
	m_iNextRequestSequence = 1u;
	m_iStateStartedAtMilliseconds = GetTickCount64();
	m_hasAuthoritativeLifecycle = false;
}

void Client::CValtanPatternAuditionService::Harness_ObserveBoss(
	const bool_t bBossValid,
	const std::string_view strPatternId,
	const uint32_t iPatternSequence)
{
	Observe_Boss(bBossValid, strPatternId, iPatternSequence);
}
#endif
