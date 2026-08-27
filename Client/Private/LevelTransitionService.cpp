#include "LevelTransitionService.h"

#include "LevelRegistry.h"
#include "NetworkManager.h"

#include <chrono>
#include <mutex>
#include <optional>

namespace
{
	std::mutex g_TransitionMutex;
	std::optional<Client::LEVEL_TRANSITION_REQUEST> g_PendingRequest;
	std::optional<Client::CLIENT_RECOVERY_DIAGNOSTIC> g_RecoveryDiagnostic;
	std::string g_Status = "No level transition is pending.";

	std::uint64_t Get_UnixMilliseconds() noexcept
	{
		using namespace std::chrono;
		return static_cast<std::uint64_t>(duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()).count());
	}
}

bool_t Client::CLevelTransitionService::Request_Load(
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	return Request(
		LEVEL_TRANSITION_PHASE::LOAD,
		eTargetLevel,
		pSource,
		lobbyCommandToken);
}

bool_t Client::CLevelTransitionService::Request_Activation(
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	return Request(
		LEVEL_TRANSITION_PHASE::ACTIVATE,
		eTargetLevel,
		pSource,
		lobbyCommandToken);
}

bool_t Client::CLevelTransitionService::Try_Consume(
	LEVEL_TRANSITION_REQUEST& outRequest)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_PendingRequest.has_value())
		return false;

	outRequest = std::move(*g_PendingRequest);
	g_PendingRequest.reset();
	g_Status = "Level transition request consumed.";
	return true;
}

bool_t Client::CLevelTransitionService::Is_Pending()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_PendingRequest.has_value();
}

std::string Client::CLevelTransitionService::Get_Status()
{
	std::scoped_lock lock{ g_TransitionMutex };
	return g_Status;
}

void Client::CLevelTransitionService::Report_LoadFailure(
	const HRESULT result,
	const std::string_view detail)
{
	Report_Recovery(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOAD_FAILED,
		"level.loading",
		detail,
		result);
}

void Client::CLevelTransitionService::Report_Recovery(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const std::string_view source,
	const std::string_view detail,
	const HRESULT result)
{
	if (!LostArk::Shared::Is_Known_SessionDiagnosticReason(reason) ||
		source.empty())
	{
		return;
	}

	std::string terminalDetail{ source };
	if (!detail.empty())
		terminalDetail += ": " + std::string{ detail };
	CNetworkManager& network = CNetworkManager::Get();
	network.Record_SessionTerminal(
		reason,
		0,
		LostArk::Shared::PACKET_TYPE::INVALID,
		terminalDetail);
	CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT session =
		network.Get_SessionDiagnosticSnapshot();
	// The process trace keeps every typed recovery edge even when the Lobby UI
	// already owns an earlier first-recovery snapshot for this generation.
	network.Record_SessionRecovery(reason, source, detail);

	{
		std::scoped_lock lock{ g_TransitionMutex };
		// The earliest terminal producer owns the root cause. Generic activation
		// cleanup may report later, but cannot erase a loader/transport detail.
		if (g_RecoveryDiagnostic.has_value())
			return;

		CLIENT_RECOVERY_DIAGNOSTIC recovery{};
		/* A validated ROOM_FULL frame is stronger than the flush-close FIN that may
		   race ahead on the receive worker. Keep the transport terminal in Session,
		   but expose the validated semantic result as the Lobby recovery reason. */
		recovery.eReason =
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_EXPECTED_ROOM_FULL ==
				reason ? reason :
			(LostArk::Shared::Is_Known_SessionDiagnosticReason(session.eReason) ?
				session.eReason : reason);
		recovery.hResult = result;
		recovery.strSource.assign(source);
		recovery.strDetail.assign(detail);
		recovery.iOccurredUnixMs = 0u != session.iTerminalUnixMs ?
			session.iTerminalUnixMs : Get_UnixMilliseconds();
		recovery.Session = std::move(session);
		g_RecoveryDiagnostic = std::move(recovery);
		g_Status = "Lobby recovery recorded by " + std::string{ source } + ".";
	}
}

void Client::CLevelTransitionService::Report_NetworkRecovery(
	const std::string_view source,
	const std::string_view detail)
{
	const CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT session =
		CNetworkManager::Get().Get_SessionDiagnosticSnapshot();
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
		LostArk::Shared::Is_Known_SessionDiagnosticReason(session.eReason) ?
			session.eReason :
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_CONNECTION_LOST;
	Report_Recovery(reason, source, detail);
}

bool_t Client::CLevelTransitionService::Try_ConsumeRecovery(
	CLIENT_RECOVERY_DIAGNOSTIC& outDiagnostic)
{
	const CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT latestSession =
		CNetworkManager::Get().Get_SessionDiagnosticSnapshot();
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_RecoveryDiagnostic.has_value())
		return false;
	if (latestSession.iConnectionGeneration ==
		g_RecoveryDiagnostic->Session.iConnectionGeneration)
	{
		g_RecoveryDiagnostic->Session = latestSession;
	}
	outDiagnostic = std::move(*g_RecoveryDiagnostic);
	g_RecoveryDiagnostic.reset();
	return true;
}

bool_t Client::CLevelTransitionService::Try_ConsumeLoadFailure(
	HRESULT& outResult,
	std::string& outDetail)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_RecoveryDiagnostic.has_value() ||
		!FAILED(g_RecoveryDiagnostic->hResult))
		return false;

	outResult = g_RecoveryDiagnostic->hResult;
	outDetail = std::move(g_RecoveryDiagnostic->strDetail);
	g_RecoveryDiagnostic.reset();
	return true;
}

Client::SERVER_WORLD_TRANSFER_PUMP_RESULT
Client::CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
	const LEVEL currentLevel)
{
	using namespace LostArk::Shared;
	CNetworkManager& networkManager = CNetworkManager::Get();
	S2C_ENTER_ACCEPTED accepted{};
	if (!networkManager.Try_Consume_EnterAccepted(accepted))
		return SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE;

	LEVEL targetLevel = LEVEL::END;
	if (NETWORK_PROTOCOL_VERSION == accepted.iProtocolVersion &&
		INVALID_PLAYER_ID != accepted.iPlayerId &&
		INVALID_NET_ENTITY_ID != accepted.iNetEntityId)
	{
		switch (accepted.eWorldId)
		{
		case WORLD_ID::BERN:
			targetLevel = LEVEL::BERN;
			break;
		case WORLD_ID::VALTAN_ARENA:
			targetLevel = LEVEL::VALTAN_ARENA;
			break;
		default:
			break;
		}
	}

	if (LEVEL::END != targetLevel && targetLevel != currentLevel &&
		Request_Load(targetLevel, "server.trigger.change-level"))
	{
		return SERVER_WORLD_TRANSFER_PUMP_RESULT::REQUESTED;
	}

	Report_Recovery(
		SESSION_DIAGNOSTIC_REASON::CLIENT_WORLD_TRANSFER_FAILED,
		"server.trigger.change-level-recovery",
		"Accepted transfer target was invalid, unchanged, or could not be staged.");
	networkManager.Close_ServerConnection();
	Request_Load(LEVEL::LOBBY, "server.trigger.change-level-recovery");
	return SERVER_WORLD_TRANSFER_PUMP_RESULT::RECOVERY_REQUESTED;
}

bool_t Client::CLevelTransitionService::Request(
	const LEVEL_TRANSITION_PHASE ePhase,
	const LEVEL eTargetLevel,
	const char_t* pSource,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	const bool_t hasLobbyCommand =
		INVALID_LOBBY_COMMAND_TOKEN != lobbyCommandToken;
	if (nullptr == CLevelRegistry::Find(eTargetLevel) ||
		nullptr == pSource || '\0' == *pSource ||
		(hasLobbyCommand && LEVEL::LOBBY != eTargetLevel))
	{
		std::scoped_lock lock{ g_TransitionMutex };
		g_Status = "Rejected invalid level transition request.";
		return false;
	}

	std::scoped_lock lock{ g_TransitionMutex };
	if (g_PendingRequest.has_value())
	{
		g_Status =
			"Rejected level transition while another request is pending.";
		return false;
	}

	g_PendingRequest = LEVEL_TRANSITION_REQUEST{
		ePhase,
		eTargetLevel,
		pSource,
		lobbyCommandToken
	};
	/* A target-level failure is reported before the recovery load back to the
	   Lobby is staged.  Clearing the report for that recovery request erased
	   the only actionable reason before CLevel_Lobby could consume it.  A new
	   non-Lobby load starts a fresh attempt and may clear the previous report;
	   the Lobby recovery itself must preserve it. */
	if (LEVEL_TRANSITION_PHASE::LOAD == ePhase &&
		LEVEL::LOBBY != eTargetLevel)
	{
		g_RecoveryDiagnostic.reset();
	}
	g_Status = "Level transition request staged by " +
		g_PendingRequest->strSource + ".";
	return true;
}
