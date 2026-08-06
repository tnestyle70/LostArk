#include "LevelTransitionService.h"

#include "LevelRegistry.h"
#include "NetworkManager.h"

#include <mutex>
#include <optional>

namespace
{
	std::mutex g_TransitionMutex;
	std::optional<Client::LEVEL_TRANSITION_REQUEST> g_PendingRequest;
	std::optional<HRESULT> g_LoadFailure;
	std::string g_Status = "No level transition is pending.";
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
	const HRESULT result)
{
	std::scoped_lock lock{ g_TransitionMutex };
	g_LoadFailure = result;
	g_Status = "Level loading failed with HRESULT " +
		std::to_string(static_cast<long>(result)) + ".";
}

bool_t Client::CLevelTransitionService::Try_ConsumeLoadFailure(
	HRESULT& outResult)
{
	std::scoped_lock lock{ g_TransitionMutex };
	if (!g_LoadFailure.has_value())
		return false;

	outResult = *g_LoadFailure;
	g_LoadFailure.reset();
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
	if (LEVEL_TRANSITION_PHASE::LOAD == ePhase)
		g_LoadFailure.reset();
	g_Status = "Level transition request staged by " +
		g_PendingRequest->strSource + ".";
	return true;
}
